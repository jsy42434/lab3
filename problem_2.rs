use std::io;

fn main() {
    let mut input = String::new();

    // 행렬의 크기 입력 받기
    println!("행렬의 행(row) 수를 입력하세요:");
    io::stdin().read_line(&mut input).unwrap();
    let rows: usize = input.trim().parse().unwrap();
    input.clear();

    println!("행렬의 열(column) 수를 입력하세요:");
    io::stdin().read_line(&mut input).unwrap();
    let cols: usize = input.trim().parse().unwrap();
    input.clear();

    // 첫 번째 행렬 입력
    println!("첫 번째 행렬의 값을 입력하세요:");
    let matrix_a = read_matrix(rows, cols);

    // 두 번째 행렬 입력
    println!("두 번째 행렬의 값을 입력하세요:");
    let matrix_b = read_matrix(rows, cols);

    // 두 행렬의 합 계산
    let matrix_c = add_matrices(&matrix_a, &matrix_b, rows, cols);

    // 결과 출력
    println!("두 행렬의 합은 다음과 같습니다:");
    print_matrix(&matrix_c, rows, cols);
}

fn read_matrix(rows: usize, cols: usize) -> Vec<Vec<i32>> {
    let mut matrix = vec![vec![0; cols]; rows];
    let mut input = String::new();

    for i in 0..rows {
        println!("행 {}의 값을 공백으로 구분하여 입력하세요:", i + 1);
        io::stdin().read_line(&mut input).unwrap();
        let row_values: Vec<i32> = input
            .trim()
            .split_whitespace()
            .map(|x| x.parse().unwrap())
            .collect();

        if row_values.len() != cols {
            panic!("열의 개수가 일치하지 않습니다!");
        }

        matrix[i] = row_values;
        input.clear();
    }

    matrix
}

fn add_matrices(
    matrix_a: &Vec<Vec<i32>>,
    matrix_b: &Vec<Vec<i32>>,
    rows: usize,
    cols: usize,
) -> Vec<Vec<i32>> {
    let mut result = vec![vec![0; cols]; rows];

    for i in 0..rows {
        for j in 0..cols {
            result[i][j] = matrix_a[i][j] + matrix_b[i][j];
        }
    }

    result
}

fn print_matrix(matrix: &Vec<Vec<i32>>, rows: usize, cols: usize) {
    for i in 0..rows {
        for j in 0..cols {
            print!("{} ", matrix[i][j]);
        }
        println!();
    }
}
