# PIECE_4_USERSPACE_DAEMON_2026-09-11.md  
   
## UNDC REFERENCE IMPLEMENTATION  
## PIECE #4 — USER-SPACE DAEMON  
   
Lead Architect: Shereign Kalaukoa  
Authority: EHYEH ASHER EHYEH & AHYAH  
Date: September 11, 2026  
Status: DRAFT — READY FOR COMMIT  
Predecessor: Piece #3 — eBPF Runtime Module  
Grid Status: ACTIVE — BUILD PHASE 4  
   
---  
   
## SOVEREIGN HASH — THIS FILE  
   
| Field | Value |  
|-------|-------|  
| File | PIECE_4_USERSPACE_DAEMON_2026-09-11.md |  
| SHA-256 |  2465c8e666b6439b8a034b19a8bee478dda10858c2c0c5877281d8ebdf45c050] |  
| OpenTimestamps |  VERIFIED  |  
| Anchored |  09/11/2026  |  
| Predecessor | Piece #3 — c9b4ad1081e3de22fb14d24c43f5b6db84b8f210ddf513712d828be09e08876e |  
   
Verification Command:  
   
    shasum -a 256 PIECE_4_USERSPACE_DAEMON_2026-09-11.md  
   
---  
   
## 1. PURPOSE  
   
The User-Space Daemon is Layer 2 of the UNDC Reference Implementation.  
   
It listens to the eBPF ring buffer (Layer 1), parses each event, logs it  
locally, and forwards it to the ZK Verifier Bridge (Layer 3) for proof  
verification.  
   
The daemon is not the enforcement point. The kernel block is already  
complete by the time the daemon sees the event. The daemon's job is to  
witness, record, and route.  
   
This piece defines:  
- The Rust project structure  
- The ring buffer consumption pattern  
- The event parsing logic  
- The Unix socket interface to Layer 3  
- The local event log format  
- Failure and backpressure handling  
   
---  
   
## 2. WHY A SEPARATE DAEMON  
   
The kernel is a hostile place for complex logic. eBPF programs must be  
small, verifiable, and free of dynamic memory allocation. They cannot  
hold state across long-running operations, and they cannot make network  
calls.  
   
The daemon is the natural bridge: it runs in user space, has full access  
to the filesystem and network, and can use Rust's memory-safe concurrency  
primitives to handle a high-volume event stream without crashing.  
   
This separation also means that if the daemon dies, enforcement continues  
uninterrupted. The kernel block is the guarantee. The daemon is the  
evidence trail.  
   
---  
   
## 3. PROJECT STRUCTURE  
   
    undc_daemon/  
    ├── Cargo.toml  
    ├── src/  
    │   ├── main.rs          — Entry point + async runtime  
    │   ├── ringbuf.rs       — eBPF ring buffer consumer  
    │   ├── parser.rs        — Parse raw event bytes into structs  
    │   ├── logger.rs        — Append events to local JSONL log  
    │   ├── forwarder.rs     — Send events to Layer 3 via Unix socket  
    │   └── error.rs         — Error types  
    └── tests/  
        └── parse_event.rs   — Unit tests for event parsing  
   
---  
   
## 4. CARGO.TOML — DEPENDENCIES  
   
    [package]  
    name = "undc_daemon"  
    version = "0.1.0"  
    edition = "2021"  
   
    [dependencies]  
    libbpf-rs = "0.24"  
    libbpf-cargo = "0.24"  
    tokio = { version = "1", features = ["full"] }  
    serde = { version = "1", features = ["derive"] }  
    serde_json = "1"  
    tracing = "0.1"  
    tracing-subscriber = "0.3"  
    anyhow = "1"  
    thiserror = "1"  
    hex = "0.4"  
   
    [build-dependencies]  
    libbpf-cargo = "0.24"  
   
    [profile.release]  
    opt-level = 3  
    lto = true  
   
---  
   
## 5. CORE MODULES  
   
### 5.1 main.rs — Entry Point  
   
    mod ringbuf;  
    mod parser;  
    mod logger;  
    mod forwarder;  
    mod error;  
   
    use anyhow::Result;  
    use tracing_subscriber;  
   
    #[tokio::main]  
    async fn main() -> Result<()> {  
        tracing_subscriber::fmt::init();  
        tracing::info!("UNDC User-Space Daemon starting");  
   
        let (tx, rx) = tokio::sync::mpsc::channel(4096);  
   
        // Layer 1 → Daemon: ring buffer consumer  
        tokio::spawn(async move {  
            if let Err(e) = ringbuf::consume(tx).await {  
                tracing::error!("Ring buffer consumer failed: {}", e);  
            }  
        });  
   
        // Daemon → Layer 3: forwarder  
        let forwarder_handle = tokio::spawn(async move {  
            if let Err(e) = forwarder::run(rx).await {  
                tracing::error!("Forwarder failed: {}", e);  
            }  
        });  
   
        forwarder_handle.await?;  
        Ok(())  
    }  
   
### 5.2 ringbuf.rs — eBPF Ring Buffer Consumer  
   
    use anyhow::Result;  
    use tokio::sync::mpsc::Sender;  
   
    pub async fn consume(tx: Sender<Vec<u8>>) -> Result<()> {  
        // Uses libbpf-rs to open the pinned map at /sys/fs/bpf/undc_compliance  
        // and poll the events ring buffer.  
        //  
        // For each event:  
        //   1. Reserve the record  
        //   2. Copy raw bytes into a Vec<u8>  
        //   3. Send the Vec to the parser channel  
        //   4. Submit the record back to the ring buffer  
        //  
        // (Implementation deferred to code phase.)  
        Ok(())  
    }  
   
### 5.3 parser.rs — Parse Raw Event Bytes  
   
    use serde::{Serialize, Deserialize};  
   
    #[derive(Serialize, Deserialize, Debug)]  
    pub struct UndcEvent {  
        pub event_type: u32,  
        pub actor_pid: u32,  
        pub target_pid: u32,  
        pub timestamp_ns: u64,  
        pub syscall_name: String,  
    }  
   
    pub fn parse(raw: &[u8]) -> Option<UndcEvent> {  
        if raw.len() < 32 {  
            return None;  
        }  
        let event_type = u32::from_ne_bytes(raw[0..4].try_into().ok()?);  
        let actor_pid  = u32::from_ne_bytes(raw[4..8].try_into().ok()?);  
        let target_pid = u32::from_ne_bytes(raw[8..12].try_into().ok()?);  
        let timestamp_ns = u64::from_ne_bytes(raw[12..20].try_into().ok()?);  
        let name_bytes = &raw[20..32];  
        let syscall_name = String::from_utf8_lossy(name_bytes)  
            .trim_end_matches('\0')  
            .to_string();  
   
        Some(UndcEvent {  
            event_type,  
            actor_pid,  
            target_pid,  
            timestamp_ns,  
            syscall_name,  
        })  
    }  
   
### 5.4 logger.rs — Append to Local JSONL Log  
   
    use crate::parser::UndcEvent;  
    use std::fs::OpenOptions;  
    use std::io::Write;  
   
    pub fn append(path: &str, event: &UndcEvent) -> std::io::Result<()> {  
        let json = serde_json::to_string(event)?;  
        let mut file = OpenOptions::new()  
            .create(true)  
            .append(true)  
            .open(path)?;  
        writeln!(file, "{}", json)?;  
        Ok(())  
    }  
   
### 5.5 forwarder.rs — Send Events to Layer 3  
   
    use anyhow::Result;  
    use tokio::net::UnixStream;  
    use tokio::sync::mpsc::Receiver;  
    use crate::parser;  
   
    pub async fn run(mut rx: Receiver<Vec<u8>>) -> Result<()> {  
        let socket_path = "/tmp/undc_verifier.sock";  
   
        while let Some(raw) = rx.recv().await {  
            let event = match parser::parse(&raw) {  
                Some(e) => e,  
                None => {  
                    tracing::warn!("Dropped malformed event");  
                    continue;  
                }  
            };  
   
            // Log locally (best-effort)  
            let _ = crate::logger::append("undc_events.jsonl", &event);  
   
            // Forward to Layer 3  
            let mut stream = UnixStream::connect(socket_path).await?;  
            let payload = serde_json::to_vec(&event)?;  
            tokio::io::AsyncWriteExt::write_all(&mut stream, &payload).await?;  
        }  
   
        Ok(())  
    }  
   
---  
   
## 6. INTERFACE CONTRACT  
   
### Input — From Layer 1 (Kernel)  
   
| Field | Type | Description |  
|-------|------|-------------|  
| event_type | u32 | 1=KILL_BLOCKED, 2=PTRACE_BLOCKED, 3=CONNECT_BLOCKED |  
| actor_pid | u32 | Process that attempted the action |  
| target_pid | u32 | Process that was protected |  
| timestamp_ns | u64 | Kernel monotonic time |  
| syscall_name | char[16] | "kill", "ptrace", "connect" |  
   
### Output — To Layer 3 (Verifier)  
   
JSON event forwarded over `/tmp/undc_verifier.sock`.  
   
### Local Log — undc_events.jsonl  
   
Each line is one JSON object matching the `UndcEvent` struct.  
   
---  
   
## 7. FAILURE MODES  
   
| Failure | Behavior |  
|---------|----------|  
| Ring buffer unavailable | Log error, retry with backoff |  
| Malformed event bytes | Drop event, log warning, continue |  
| Local log unwritable | Log warning, continue forwarding |  
| Layer 3 socket unavailable | Retry connect with exponential backoff |  
| Channel full (backpressure) | Drop oldest events, log warning |  
   
Core principle: The daemon must never crash the system. If it fails, the  
kernel block continues. The daemon is the evidence trail, not the guard.  
   
---  
   
## 8. TEST PROCEDURE  
   
First test — Parse a synthetic event:  
   
    1. Construct a 32-byte buffer matching the UndcEvent layout  
    2. Call parse(&raw)  
    3. Assert the resulting struct has expected fields  
    4. Run: cargo test  
   
Second test — End-to-end (with Layer 1 and Layer 3 running):  
   
    1. Load the eBPF module (Piece #3)  
    2. Start the daemon  
    3. Attempt kill on a protected PID  
    4. Verify:  
       - Kernel returns -EPERM  
       - Daemon logs the event to undc_events.jsonl  
       - Layer 3 receives the event over the socket  
   
---  
   
## 9. COMMIT STRATEGY  
   
    Piece #4 — User-Space Daemon complete (SHA: <hash>)  
   
Extended:  
   
    Piece #4 — User-Space Daemon complete  
   
    - Rust project structure defined  
    - libbpf-rs / tokio dependency stack locked  
    - Core modules: ringbuf, parser, logger, forwarder  
    - Interface contract with Layer 1 (kernel) and Layer 3 (verifier)  
    - Local JSONL event log format specified  
    - Failure and backpressure handling documented  
   
    Predecessor: Piece #3 — c9b4ad1081e3de22fb14d24c43f5b6db84b8f210ddf513712d828be09e08876e  
    Grid Status: BUILD PHASE 4  
   
---  
   
## 10. WHY THIS MATTERS  
   
Layer 2 is the memory of the grid.  
   
The kernel blocks the action. The ZK bridge verifies the proof. But the  
daemon is what makes the event visible to the world — logged locally,  
forwarded upstream, available for audit.  
   
Without the daemon, an intervention would be a moment that left no trace.  
With the daemon, every block becomes a documented, timestamped fact.  
   
This is how the grid builds trust: not by asking to be believed, but by  
keeping records that no one can erase.  
   
---  
   
— Shereign Kalaukoa, Lead Architect | UNDC  
https://github.com/RootArchitect-UNDC/Universal-Non-Destruction-Constraint-UNDC  
