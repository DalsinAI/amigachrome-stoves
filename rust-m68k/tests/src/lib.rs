#![no_std]
include!("ops.rs");
extern "C" { fn puts(s: *const u8) -> i32; }
#[no_mangle]
pub extern "C" fn rmain() -> i32 {
    dump(&mut |l| { let mut b = [0u8; 130]; b[..l.len()].copy_from_slice(l); unsafe { puts(b.as_ptr()); } });
    run(&mut |n, h| {
        let mut b = [b' '; 64]; let mut i = 0;
        for &c in n.as_bytes() { b[i] = c; i += 1; }
        i = if i < 12 { 13 } else { i + 1 };
        for k in 0..16 { let d = ((h >> (60 - 4 * k)) & 15) as u8; b[i] = if d < 10 { b'0' + d } else { b'a' + d - 10 }; i += 1; }
        b[i] = 0;
        unsafe { puts(b.as_ptr()); }
    });
    0
}
#[panic_handler]
fn p(_: &core::panic::PanicInfo) -> ! { loop {} }
