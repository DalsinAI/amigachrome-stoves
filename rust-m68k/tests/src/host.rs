include!("ops.rs");
fn main() { dump(&mut |l| println!("{}", std::str::from_utf8(l).unwrap())); run(&mut |n, h| println!("{:<12} {:016x}", n, h)); }
