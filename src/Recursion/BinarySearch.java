package Recursion;

public class BinarySearch {
    public static void main(String[] args) {
        int[] arr = {1, 4, 5, 7, 56, 97, 100};
        int target = 4;
        int s = 0;
        int e = arr.length - 1;
        System.out.println(search(arr, target, s, e));
    }
    static int search(int[]arr, int target, int s, int e) {
        if(s > e){
            return -1;
        }
        int mid = (s + e) / 2;

        if(arr[mid] == target){
            return mid;
        }
        else if(arr[mid] > target){
            return search(arr,target,s,mid-1);
        }
        return search(arr,target,mid+1,e);
    }
}
