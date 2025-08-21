import java.util.*;
public class Peak {
    public static void main(String[] args) {
        Scanner sc = new Scanner(System.in);
        int n = sc.nextInt();
        int[] arr = new int[n];
        for(int i = 0; i < n; i++) {
            arr[i] = sc.nextInt();
        }
        int ans = peakIndex(arr);
        System.out.println(ans);
    }
    static  int peakIndex(int[] arr){
        if(arr.length == 1){
            return 0;
        }
        int start = 0;
        int end = arr.length - 1;
        while(start <=  end){
            int mid = (start + end) / 2;
            if(arr[mid] > arr[mid+1] && arr[mid] > arr[mid-1]){
                return mid;
            }
            else if(arr[mid] > arr[mid-1]) {
                start = mid + 1;
            }
            else{
                end = mid - 1;
            }
        }
        return -1;
    }
}
