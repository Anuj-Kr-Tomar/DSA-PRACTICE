import java.util.*;
public class prefixSum {
    public static void main(String[] args) {
        Scanner sc = new Scanner(System.in);
        int n = sc.nextInt();
        int []arr =  new int[n];
        for(int i = 0; i < n; i++) {
            arr[i] = sc.nextInt();
        }
        sort_arr(arr);
        for(int i = 0; i < n; i++){
            System.out.print(arr[i] + " ");
        }
    }

    static void sort_arr(int[] arr){
        int l = 0;
        int r = arr.length - 1;
        while(l < r) {
            if(arr[l] == 1 && arr[r] == 0) {
                swap(arr, l, r);
                l++;
                r--;
            }
            else if(arr[l] == 0){
                l++;
            }
            else if (arr[r] == 1) {
                r--;
            }
        }
    }
    static void swap(int[] arr, int i, int j) {
        int temp = arr[i];
        arr[i] = arr[j];
        arr[j] = temp;
    }
}
