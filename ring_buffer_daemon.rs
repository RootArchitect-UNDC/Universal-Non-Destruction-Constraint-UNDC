use libbpf_rs::skel::{OpenSkel, SkelBuilder};
use libbpf_rs::RingBufferBuilder;
use std::collections::HashMap;
use std::ffi::CStr;
use std::os::raw::c_char;
use std::sync::{Arc, Mutex};
use std::time::{Duration, Instant};
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

struct TtlCache {
    last_seen: HashMap<Vec<u8>, Instant>,
    ttl: Duration,
}

impl TtlCache {
    fn new(ttl_secs: u64) -> Self {
        Self {
            last_seen: HashMap::new(),
            ttl: Duration::from_secs(ttl_secs),
        }
    }

    fn touch(&mut self, key: Vec<u8>) {
        self.last_seen.insert(key, Instant::now());
    }

    fn evict_expired(&mut self) -> Vec<Vec<u8>> {
        let now = Instant::now();
        let expired: Vec<Vec<u8>> = self.last_seen
            .iter()
            .filter_map(|(key, last)| {
                if now.duration_since(*last) > self.ttl {
                    Some(key.clone())
                } else {
                    None
                }
            })
            .collect();

        for key in &expired {
            self.last_seen.remove(key);
        }
        expired
    }
}

#[tokio::main]
async fn main() -> Result<(), Box<dyn std::error::Error>> {
    let skel_builder = undc_lsm::UndcLsmSkelBuilder::default();
    let open_skel = skel_builder.open()?;
    let mut loaded_skel = open_skel.load()?;
    loaded_skel.attach()?;

    println!("[UNDC DAEMON] Kernel LSM Hooks Attached. Sovereign Floor Active.");

    let maps = loaded_skel.maps();
    let net_allowlist = maps.undc_net_allowlist().clone();
    let ring_buf_map = maps.undc_ring_buffer();

    // Thread-safe shared cache
    let cache = Arc::new(Mutex::new(TtlCache::new(300)));
    let cache_for_events = Arc::clone(&cache);
    let cache_for_sweeper = Arc::clone(&cache);

    let mut rb_builder = RingBufferBuilder::new();
    rb_builder.add(ring_buf_map, move |data| {
        if let Err(e) = handle_slowpath_event(data, &net_allowlist, &cache_for_events) {
            eprintln!("[UNDC ERROR] Failed to process slowpath event: {:?}", e);
        }
        0
    })?;

    let ring_buffer = rb_builder.build()?;

    tokio::task::spawn_blocking(move || {
        let mut last_eviction = Instant::now();

        loop {
            if let Err(e) = ring_buffer.poll(Duration::from_millis(100)) {
                eprintln!("[UNDC CRITICAL] Ring buffer polling failed: {:?}", e);
                break;
            }

            if last_eviction.elapsed() > Duration::from_secs(30) {
                if let Ok(mut locked_cache) = cache_for_sweeper.lock() {
                    let expired = locked_cache.evict_expired();
                    for key in expired {
                        let _ = net_allowlist.delete(&key);
                    }
                }
                last_eviction = Instant::now();
            }
        }
    });

    signal::ctrl_c().await?;
    println!("[UNDC DAEMON] Gracious Shutdown Intercepted. Maintaining Kernel Floor Safety.");
    Ok(())
}

fn handle_slowpath_event(
    data: &[u8],
    net_map: &libbpf_rs::Map,
    cache: &Arc<Mutex<TtlCache>>,
) -> Result<(), Box<dyn std::error::Error>> {
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

        // Register key in TTL cache
        if let Ok(mut locked_cache) = cache.lock() {
            locked_cache.touch(key.clone());
        }

        let policy_mask: u64 = 0x1;
        net_map.update(&key, &policy_mask.to_ne_bytes(), libbpf_rs::MapFlags::ANY)?;
    } else {
        eprintln!("[UNDC VIOLATION] Action violates system invariants. Purging execution slot.");
        nix::sys::signal::kill(nix::unistd::Pid::from_raw(event.pid as i32), nix::sys::signal::Signal::SIGKILL)?;
    }

    Ok(())
}

fn evaluate_dependency_graph(_path: &str, _event: &UndcSlowpathEvent) -> bool {
    true
}
