use aya::maps::RingBuf;
use aya::programs::Lsm;
use aya::Bpf;
use std::fs::File;
use std::io::Write;
use std::process::Command;
use std::convert::TryFrom;

#[repr(C)]
#[derive(Clone, Copy)]
struct Event {
    timestamp_ns: u64,
    delta_ns: u64,
    f_measured: u32,
    cpu_id: u32,
}

fn main() -> Result<(), Box<dyn std::error::Error>> {
    let mut bpf = Bpf::load_file("fml_kernel.o")?;
    
    let program: &mut Lsm = bpf.program_mut("check_frequency_invariant")?.try_into()?;
    program.load()?;
    program.attach()?;

    println!("[*] Sovereign FML kernel driver attached safely to ring architecture.");
    
    let ring_buffer = RingBuf::try_from(bpf.map_mut("fml_ringbuf")?)?;
    
    loop {
        if let Some(item) = ring_buffer.read() {
            let event = unsafe { &*(item.as_ptr() as *const Event) };
            
            println!("[Kernel Event] CPU: {}, Freq: {} Hz", event.cpu_id, event.f_measured);

            if let Err(e) = execute_proof_pipeline(event.f_measured) {
                eprintln!("[Error] Pipeline transformation exception: {:?}", e);
            }
        }
    }
}

fn execute_proof_pipeline(f_measured: u32) -> std::io::Result<()> {
    let blinding_factor = "99999"; 
    let epsilon = "5";
    let expected_root = "12903517409275039572620703862215160867086088289457630713791880495393433602161";

    let payload = format!(
        r#"{{"expected_root": "{}", "epsilon": "{}", "f_measured": "{}", "blinding_factor": "{}"}}"#,
        expected_root, epsilon, f_measured, blinding_factor
    );

    let mut file = File::create("input.json")?;
    file.write_all(payload.as_bytes())?;

    let output = Command::new("./prove_fml.sh").output()?;
    
    if output.status.success() {
        println!("✅ Proof Cycle Confirmed: State conforms to 432 Hz tracking constraints.");
    } else {
        eprintln!("❌ Warning: State proof failed validation constraints.");
    }

    Ok(())
}
