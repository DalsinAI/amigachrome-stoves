// Shared by the host and the m68k build: integer ops on pseudo-random inputs,
// one checksum per op.
pub struct Rng(pub u64);
impl Rng { pub fn next(&mut self) -> u64 { let mut x = self.0; x ^= x << 13; x ^= x >> 7; x ^= x << 17; self.0 = x; x } }
#[inline(never)] fn bb<T>(x: T) -> T { core::hint::black_box(x) }
fn mix(h: &mut u64, v: u64) { *h = (*h ^ v).wrapping_mul(0x100000001b3); }
pub struct HashW(pub u64, pub u32);
impl core::fmt::Write for HashW { fn write_str(&mut self, s: &str) -> core::fmt::Result { for &c in s.as_bytes() { mix(&mut self.0, c as u64); self.1 += 1; } Ok(()) } }
pub fn run(out: &mut dyn FnMut(&str, u64)) {
    use core::fmt::Write;
    for prec in [0usize, 1, 2, 3, 5, 8, 12, 17] {
        let mut r = Rng(0x1234567); let mut w = HashW(0xcbf29ce484222325, 0);
        for _ in 0..300 { let f = ((r.next() >> 20) as f64) / 1000.0; let _ = write!(w, "{:.*}|", prec, bb(f)); }
        out(["fmt_exact.0","fmt_exact.1","fmt_exact.2","fmt_exact.3","fmt_exact.5","fmt_exact.8","fmt_exact.12","fmt_exact.17"][[0usize,1,2,3,5,8,12,17].iter().position(|&p| p==prec).unwrap()], w.0);
    }
    { let mut r = Rng(77); let mut w = HashW(0xcbf29ce484222325, 0);
      for _ in 0..300 { let f = f64::from_bits(r.next() & 0x7fef_ffff_ffff_ffff); let _ = write!(w, "{}|{:e}|", bb(f), f); } out("fmt_short", w.0); }
    { let mut r = Rng(78); let mut w = HashW(0xcbf29ce484222325, 0);
      for _ in 0..300 { let f = f32::from_bits(r.next() as u32 & 0x7f7f_ffff); let _ = write!(w, "{}|{:.4e}|", bb(f), f); } out("fmt_f32", w.0); }
    macro_rules! op { ($name:expr, $n:expr, |$r:ident| $e:expr) => {{
        let mut $r = Rng(0x9E3779B97F4A7C15); let mut h = 0xcbf29ce484222325u64;
        for _ in 0..$n { let v: u64 = $e; mix(&mut h, v); }
        out($name, h);
    }} }
    op!("u32 add", 2000, |r| (bb(r.next() as u32).wrapping_add(r.next() as u32)) as u64);
    op!("u32 mul", 2000, |r| (bb(r.next() as u32).wrapping_mul(r.next() as u32)) as u64);
    op!("u32 div", 2000, |r| { let a = bb(r.next() as u32); let b = (r.next() as u32 >> (r.next() % 32)) | 1; (a / b) as u64 });
    op!("u32 rem", 2000, |r| { let a = bb(r.next() as u32); let b = (r.next() as u32 >> (r.next() % 32)) | 1; (a % b) as u64 });
    op!("i32 div", 2000, |r| { let a = bb(r.next() as i32); let b = ((r.next() as i32) >> (r.next() % 31)) | 1; a.wrapping_div(b) as u64 });
    op!("i32 rem", 2000, |r| { let a = bb(r.next() as i32); let b = ((r.next() as i32) >> (r.next() % 31)) | 1; a.wrapping_rem(b) as u64 });
    op!("u32 shl", 2000, |r| (bb(r.next() as u32) << (r.next() % 32)) as u64);
    op!("u32 shr", 2000, |r| (bb(r.next() as u32) >> (r.next() % 32)) as u64);
    op!("i32 sar", 2000, |r| (bb(r.next() as i32) >> (r.next() % 32)) as u64);
    op!("u32 mulhi", 2000, |r| ((bb(r.next() as u32) as u64 * (r.next() as u32) as u64) >> 32));
    op!("u16 ops", 2000, |r| { let a = bb(r.next() as u16); let b = (r.next() as u16) | 1; (a.wrapping_mul(b) ^ (a / b) ^ (a % b) ^ a.rotate_left(3)) as u64 });
    op!("u8 ops", 2000, |r| { let a = bb(r.next() as u8); let b = (r.next() as u8) | 1; (a.wrapping_mul(b) ^ (a / b) ^ (a % b)) as u64 });
    op!("u64 add", 2000, |r| bb(r.next()).wrapping_add(r.next()));
    op!("u64 sub", 2000, |r| bb(r.next()).wrapping_sub(r.next()));
    op!("u64 mul", 2000, |r| bb(r.next()).wrapping_mul(r.next()));
    op!("u64 div", 2000, |r| { let a = bb(r.next()); let b = (r.next() >> (r.next() % 64)) | 1; a / b });
    op!("u64 rem", 2000, |r| { let a = bb(r.next()); let b = (r.next() >> (r.next() % 64)) | 1; a % b });
    op!("i64 div", 2000, |r| { let a = bb(r.next() as i64); let b = ((r.next() as i64) >> (r.next() % 63)) | 1; a.wrapping_div(b) as u64 });
    op!("u64 shl", 2000, |r| bb(r.next()) << (r.next() % 64));
    op!("u64 shr", 2000, |r| bb(r.next()) >> (r.next() % 64));
    op!("i64 sar", 2000, |r| (bb(r.next() as i64) >> (r.next() % 64)) as u64);
    op!("u64 cmp", 2000, |r| { let a = bb(r.next()); let b = r.next() >> (r.next() % 3); (a < b) as u64 | ((a == b) as u64) << 1 | (((a as i64) < (b as i64)) as u64) << 2 });
    op!("u64 mulhi", 2000, |r| ((bb(r.next()) as u128 * r.next() as u128) >> 64) as u64);
    op!("u128 mul", 1000, |r| { let a = ((bb(r.next()) as u128) << 64) | r.next() as u128; let b = ((r.next() as u128) << 64) | r.next() as u128; let p = a.wrapping_mul(b); (p ^ (p >> 64)) as u64 });
    op!("u128 div", 1000, |r| { let a = ((bb(r.next()) as u128) << 64) | r.next() as u128; let b = ((((r.next() as u128) << 64) | r.next() as u128) >> (r.next() % 128)) | 1; let q = a / b; let m = a % b; (q ^ (q >> 64) ^ m ^ (m >> 64)) as u64 });
    op!("u64 clz/ctz", 2000, |r| { let a = bb(r.next()) >> (r.next() % 64); (a.leading_zeros() as u64) | (a.trailing_zeros() as u64) << 8 | (a.count_ones() as u64) << 16 | (a.swap_bytes() ^ a.reverse_bits()) << 24 });
    op!("u32 clz/ctz", 2000, |r| { let a = bb(r.next() as u32) >> (r.next() % 32); (a.leading_zeros() as u64) | (a.trailing_zeros() as u64) << 8 | (a.count_ones() as u64) << 16 | ((a.swap_bytes() ^ a.reverse_bits()) as u64) << 24 });
    op!("f64 arith", 2000, |r| { let a = f64::from_bits(bb(r.next()) & 0x7fef_ffff_ffff_ffff ^ (r.next() & 1 << 63)); let b = (r.next() >> 11) as f64 / 3.0; (a + b).to_bits() ^ (a * b).to_bits().rotate_left(7) ^ (a / (b + 1.0)).to_bits().rotate_left(13) ^ (b - a).to_bits().rotate_left(29) });
    op!("f64 conv", 2000, |r| { let x = bb(r.next()); let f = (x as i64 >> (x % 50)) as f64; (f.to_bits()) ^ ((f * 0.7) as i64 as u64).rotate_left(5) ^ ((((x >> 40) as u32 as f64) * 1.0001).to_bits()).rotate_left(11) });
    op!("f32 arith", 2000, |r| { let a = (bb(r.next()) >> 40) as f32 / 7.0; let b = (r.next() as u32 >> 8) as f32 * 0.001; ((a + b).to_bits() ^ (a * b).to_bits().rotate_left(7) ^ (a / (b + 1.0)).to_bits().rotate_left(13)) as u64 ^ ((a as f64 * 3.3) as f32).to_bits() as u64 });
    op!("f64 cmp", 2000, |r| { let a = (bb(r.next()) as i64 >> 20) as f64; let b = (r.next() as i64 >> 20) as f64; (a < b) as u64 | ((a == b) as u64) << 1 | ((a >= b) as u64) << 2 | ((if a > b { a } else { b }).to_bits() ^ (if a < b { a } else { b }).to_bits()) << 3 });
    op!("u64->f64", 2000, |r| ((bb(r.next()) >> (r.next() % 64)) as f64).to_bits());
    op!("u32->f64", 2000, |r| ((bb(r.next() as u32) >> (r.next() % 32)) as f64).to_bits());
    op!("f64->u64", 2000, |r| { let f = f64::from_bits(bb(r.next()) & 0x43ef_ffff_ffff_ffff); f as u64 });
    op!("f64 div", 2000, |r| { let a = (bb(r.next()) >> 20) as i64 as f64; let b = ((r.next() >> 40) | 1) as i64 as f64; (a / b).to_bits() });
    op!("u64->f64/1000", 2000, |r| (((bb(r.next()) >> 20) as f64) / 1000.0).to_bits());
    op!("bool/select", 2000, |r| { let a = bb(r.next()); let c = a & 7; let v = match c { 0 => a >> 3, 1 => a.wrapping_mul(3), 2 => !a, 3 => a ^ 0x5555, 4 => a.rotate_right(9), 5 => a / 7, _ => a.wrapping_neg() }; if (a as i32) < 0 { v } else { v ^ 1 } });
}
pub struct LineW { pub b: [u8; 128], pub n: usize }
impl core::fmt::Write for LineW { fn write_str(&mut self, s: &str) -> core::fmt::Result { for &c in s.as_bytes() { if self.n < 127 { self.b[self.n] = c; self.n += 1; } } Ok(()) } }
pub fn dump(line: &mut dyn FnMut(&[u8])) {
    use core::fmt::Write;
    let mut r = Rng(0x1234567);
    for _ in 0..40 { let f = ((r.next() >> 20) as f64) / 1000.0; let mut w = LineW { b: [0; 128], n: 0 }; let _ = write!(w, "{:?} {:.0} {:.3} {:.8}", f.to_bits(), bb(f), f, f); line(&w.b[..w.n]); }
}
