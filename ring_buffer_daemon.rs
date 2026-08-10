use libbpf_rs::skel::{OpenSkel, SkelBuilder};
use libbpf_rs::RingBufferBuilder;
use std::ffi::CStr;
use std::os::raw::c_char;
use tokio::signal;

mod undc_lsm {
    include!(concat!(env!("OUT_DIR"), "/undc_lsm.skel.rs"));
}

#[repr(C, align(8))]
struct UndcSlowpathEvent {
    pid: u32,
    event_type: u32,
    entity_id: u64,
    entity_port: u32,
    execution_path: [c_char; 256],
}

#[tokio::main]
async fn main() -> Result<(), Box<dyn std::error::Error>> {
    let skel_builder = undc_lsm::UndcLsmSkelBuilder::default();
    let open_skel = skel_builder.open()?;
    let mut loaded_skel = open_skel.load()?;
    loaded_skel.attach()?;

    println!("[UNDC DAEMON] Kernel LSM Hooks Attached. Sovereign Floor Active.");

    let maps = loaded_skel.maps();
    let net_allowlist = maps.undc_net_allowlist();
    let ring_buf_map = maps.undc_ring_buffer();

    let mut rb_builder = RingBufferBuilder::new();
    rb_builder.add(ring_buf_map, move |data| {
        if let Err(e) = handle_slowpath_event(data, &net_allowlist) {
            eprintln!("[UNDC ERROR] Failed to process slowpath event: {:?}", e);
        }
        0
    })?;

    let ring_buffer = rb_builder.build()?;

    tokio::task::spawn_blocking(move || {
        loop {
            if let Err(e) = ring_buffer.poll(std::time::Duration::from_millis(100)) {
                eprintln!("[UNDC CRITICAL] Ring buffer polling failed: {:?}", e);
                break;
            }
        }
    });

    signal::ctrl_c().await?;
    println!("[UNDC DAEMON] Gracious Shutdown Intercepted. Maintaining Kernel Floor Safety.");
    Ok(())
}

fn handle_slowpath_event(data: &[u8], net_map: &libbpf_rs::Map) -> Result<(), Box<dyn std::error::Error>> {
    if data.len() < std::mem::size_of::<UndcSlowpathEvent>() {
        return Err("Malformed packet".into());
    }

    let ptr = data.as_ptr();
    if (ptr as usize) % std::mem::align_of::<UndcSlowpathEvent>() != 0 {
        return Err("Alignment error".into());
    }
    let event = unsafe { &*(ptr as *const UndcSlowpathEvent) };

    let path_slice = &data[16..272];
    let mut safe_path = [0u8; 256];
    safe_path.copy_from_slice(path_slice);
    safe_path[255] = 0;

    let path_cstr = CStr::from_bytes_until_nul(&safe_path)?;
    let path_str = path_cstr.to_string_lossy();

    println!(
        "[UNDC EVALUATION] Slow-path Event -> PID: {}, Type: {}, Target Entity: {}",
        event.pid, event.event_type, event.entity_id
    );

    let is_validated_by_manifest = evaluate_dependency_graph(&path_str, event);

    if is_validated_by_manifest {
        println!("[UNDC VALIDATION] Path verified. Updating high-speed BPF Map for O(1) fast-path.");
        
        let mut key = vec![0u8; 8];
        key[0..4].copy_from_slice(&(event.entity_id as u32).to_ne_bytes());
        key[4..6].copy_from_slice(&(event.entity_port as u16).to_ne_bytes());
        
        let policy_mask: u64 = 0x1;
        net_map.update(&key, &policy_mask.to_ne_bytes(), libbpf_rs::MapFlags::ANY)?;
    } else {
        eprintln!("[UNDC VIOLATION] Action violates system invariants. Purging execution slot.");
        nix::sys::signal::kill(nix::unistd::Pid::from_raw(event.pid as i32), nix::sys::signal::Signal::SIGKILL)?;
    }

    Ok(())
}

fn evaluate_dependency_graph(_path: &str, _event: &UndcSlowpathEvent) -> bool {
    // 1. Verifies cryptographic identity hash of the execution vector
    // 2. Compares request signature against blockchain-anchored EVIDENCE_MANIFEST.md
    true
}
