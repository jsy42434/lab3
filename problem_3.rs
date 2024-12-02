use std::fs;
use std::fs::File;
use std::io::{self, Write, Read};
use std::path::Path;

fn main() -> io::Result<()> {
    // 1. 디렉토리 생성
    let dir_name = "example_dir";
    if !Path::new(dir_name).exists() {
        fs::create_dir(dir_name)?;
        println!("디렉토리 '{}'가 생성되었습니다.", dir_name);
    } else {
        println!("디렉토리 '{}'는 이미 존재합니다.", dir_name);
    }

    // 2. 파일 생성 및 쓰기
    let file_path = format!("{}/example_file.txt", dir_name);
    let mut file = File::create(&file_path)?;
    writeln!(file, "이것은 Rust로 작성된 테스트 파일입니다.")?;
    writeln!(file, "파일 및 디렉토리 작업 연습 중입니다.")?;
    println!("파일 '{}'에 데이터가 작성되었습니다.", file_path);

    // 3. 파일 읽기
    let mut file = File::open(&file_path)?;
    let mut contents = String::new();
    file.read_to_string(&mut contents)?;
    println!("파일 '{}'의 내용:\n{}", file_path, contents);

    // 4. 디렉토리 내 파일 목록 출력
    println!("디렉토리 '{}'의 파일 목록:", dir_name);
    let entries = fs::read_dir(dir_name)?;
    for entry in entries {
        let entry = entry?;
        let path = entry.path();
        if path.is_file() {
            println!("- {}", path.display());
        }
    }

    // 5. 파일 삭제
    fs::remove_file(&file_path)?;
    println!("파일 '{}'이 삭제되었습니다.", file_path);

    // 6. 디렉토리 삭제
    fs::remove_dir(dir_name)?;
    println!("디렉토리 '{}'가 삭제되었습니다.", dir_name);

    Ok(())
}
