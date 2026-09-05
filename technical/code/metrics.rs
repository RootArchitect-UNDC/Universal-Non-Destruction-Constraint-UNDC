use lazy_static::lazy_static;
use prometheus::{register_int_counter, register_int_gauge, IntCounter, IntGauge};

lazy_static! {
    pub static ref UNDC_SLOWPATH_EVENTS_TOTAL: IntCounter = register_int_counter!(
        "undc_slowpath_events_total",
        "Total intercepted system actions"
    ).unwrap();

    pub static ref UNDC_ACTIVE_MAP_ENTRIES: IntGauge = register_int_gauge!(
        "undc_active_map_entries",
        "Current validated O(1) entries"
    ).unwrap();

    pub static ref UNDC_POLICY_VIOLATIONS_TOTAL: IntCounter = register_int_counter!(
        "undc_policy_violations_total",
        "Total SIGKILL violations"
    ).unwrap();

    pub static ref UNDC_CACHE_EVICTIONS_TOTAL: IntCounter = register_int_counter!(
        "undc_cache_evictions_total",
        "Total evicted stale paths"
    ).unwrap();
}
