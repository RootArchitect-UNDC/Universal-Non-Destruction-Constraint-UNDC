# Canary Alerts Log

This log documents triggers of the canary tokens embedded in `README.md` and `CONTRIBUTING.md`. Timestamps are in UTC.

| # | Date/Time (UTC) | Source Type / Provider | User Agent | Interpretation |
|---|----------------|------------------------|------------|----------------|
| 1 | 2026-06-03 12:08 | Google data center (non‑cloud) – Google LLC | `Mozilla/5.0 (Linux; Android 13; Pixel 4a) … GoogleApp/… AppEngine-Google; appid: s~virustotalcloud` | **Google VirusTotal scanner** – automated security scan. |
| 2 | 2026-06-08 05:40 | Data center (New York) – Datacamp | `python-requests/2.31.0` | **Automated script** – likely scanner or VPN exit node. |
| 3 | 2026-06-08 00:13 | Cloud VPS – DigitalOcean | `Mozilla/5.0 (Windows NT 6.1; Win64; x64) Chrome/60.0.3112.90` | **Bot or automated request** – outdated browser on cloud VPS. |
| 4 | 2026-06-04 15:08 | Residential ISP – A1 Telekom Austria | `Mozilla/5.0 (Windows NT 10.0; Win64; x64) Chrome/130.0.0.0` | **Real person** (likely journalist, researcher) in Austria. |
| 5 | 2026-06-08 00:23 | Commercial VPN – Avast | (not recorded) | **Privacy‑conscious visitor** using commercial VPN. |
| 6 | 2026-06-08 00:24 | Microsoft data center | (not recorded) | **Automated scan** – likely Bingbot or Azure security scanner. |
| 7 | 2026-06-08 00:30 | Malicious C2 server – Verizon Business | (not recorded) | **Adversarial reconnaissance** – known malicious IP; reported to Verizon (ref #2026060802747). |
| 8 | 2026-06-08 02:18 | Commercial VPN – Private Internet Access (PIA) | (not recorded) | **VPN user** – privacy‑conscious visitor. |
| 9 | 2026-06-08 03:05 | Corporate gateway / VPN – Fortinet / TELUS | (not recorded) | **Corporate gateway or VPN** – enterprise network. |
| 10 | 2026-06-08 03:33 | AWS data center – Amazon EC2 | (not recorded) | **Automated scanner or cloud‑hosted script**. |
| 11 | 2026-06-08 05:34 | Google Cloud Platform (GCP) | (not recorded) | **Result of seeding protocol** – Google infrastructure fetching the canary. |

All alerts are routine monitoring. No breach or harm has occurred.
