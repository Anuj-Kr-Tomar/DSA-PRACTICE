import java.util.Scanner;
public class coding {
    public static void main(String[] args) {
        Scanner sc = new Scanner(System.in);
        int [][]mat = new int[5][5];
        for (int i = 0; i < 5; i++) {
            for (int j = 0; j < 5; j++) {
                mat[i][j] = sc.nextInt();
            }
        }
        for (int i = 0; i < 5; i++) {
            for (int j = 0; j < 5; j++) {
                if (mat[i][j] == 1) {
                    System.out.println(Math.abs(2 - i) + Math.abs(2-j));
                    break;
                }
            }
        }

    }
}
