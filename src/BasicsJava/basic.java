package BasicsJava;

import java.util.Scanner;
public class basic {
    public static void main(String[] args) {
        Scanner sc = new Scanner(System.in);
        int day = 3;
        switch (day){
            case 1:
                System.out.println("today is day 1");
                break;
            case 2:
                System.out.println("today is day 2");
                break;
            default:
                System.out.println("idk");
        }
       }
}
