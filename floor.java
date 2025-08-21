public class floor {
    public static void main(String[] args) {
       int[] arr = {2, 5 , 7, 10, 12, 23};
       int target = 23;
       int ans = floornum(arr, target);
       System.out.println(ans);
    }

    static int floornum(int[] arr, int target) {
        int start = 0;
        int end = arr.length - 1;
        int ans = -1;
        while(start <= end) {
            int mid = (start +end) / 2;
            if(arr[mid] == target){
                return arr[mid];
            }
            else if(arr[mid] > target) {   
                end  = mid - 1;
            }
            else {
                ans = arr[mid];
                start = mid + 1;     
            }
        }
        return ans;
    }
} 
