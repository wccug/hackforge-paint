

# Hackforge paint XDG Portal Wrapper

This rust library generates a dylib providing hackforge paint a way to perform common OS actions (EG opening a file picker) via XDG Portal.

Building:

- Install rust with nightly toolchain (rustup recommended)
- Run `cargo build --artifact-dir ../build --release -Z unstable-options`
