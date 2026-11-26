//! Data, licensing and content packages for MDN Scan (GDExtension).

/// Version of this crate (kept in sync with the release by release-please).
#[must_use]
pub fn version() -> &'static str {
    env!("CARGO_PKG_VERSION")
}

#[cfg(test)]
mod tests {
    use super::version;

    #[test]
    fn version_is_semver() {
        let parts: Vec<u32> = version().split('.').map(|p| p.parse().unwrap()).collect();
        assert_eq!(parts.len(), 3);
    }
}
