#ifndef QSPI_PROBE_H
#define QSPI_PROBE_H

/*
 * QSPI linear-port characterization (probe_*). Reads the port through the
 * non-cacheable flash alias at 0xFB200000 (single-beat) or through the
 * cached XIP window after L1D+L2 invalidate (linefill bursts), CRC32s the
 * whole image per candidate config, and lets measurement - not assumption -
 * decide which port configuration is trustworthy. This is the only
 * legitimate prerequisite for removing xip_warm_cache.
 *
 * Console commands (polled in main()):
 *   'q'  single-beat sweep: (BAUD_RATE_DIV x DLY) via the alias
 *   'b'  burst sweep: BAUD_RATE_DIV at golden DLY via the cached window
 *   'c'  print the BootROM-era QSPI register snapshot (known-good config)
 *   'r'  replicate the BootROM config and climb BAUD_RATE_DIV from it
 *   'n'  direct-sampling sweep (LPBK=0) over legal REF/BAUD combinations
 *   'm'  word-by-word alias-vs-cached mismatch scan at the legal config
 *   'e'  go-live: switch to the legal config, evict caches, stay (no restore)
 *
 * Each config is tested in an IRQ-masked window of a few ms; USB stalls,
 * so do not run sweeps during a CDC throughput measurement. If a burst
 * test leaves the golden burst path broken, the restore-path printf may
 * never appear - a frozen LED is the answer; power-cycle to recover.
 */

/* Single-beat sweep via the alias: one "div dly crc32" line per config. */
void probe_sweep(void);

/* Burst (linefill) sweep via the cached window: one line per divider. */
void probe_burst(void);

/* Print the BootROM-era QSPI register snapshot. */
void probe_boot_cfg(void);

/* Replicate the BootROM config, then climb BAUD_RATE_DIV from it. */
void probe_bootrom(void);

/* Direct-sampling sweep (LPBK=0) over legal REF/BAUD combinations. */
void probe_direct(void);

/* Word-by-word alias-vs-cached mismatch scan at the legal config. */
void probe_mismatch(void);

/* Per-1KB-block CRC32 dump for host-side flash-vs-file localization. */
void probe_blocks(void);

/* Switch to the legal config, evict all caches, and keep running on it. */
void probe_golive(void);

/* Nonzero once probe_golive() ran: probe commands restore the dead golden
   config and would hang the board, so main must stop dispatching them. */
int probe_keys_frozen(void);

#endif /* QSPI_PROBE_H */
