package staticExample;

public class staticBlock {
    static int a = 4;
    static int b;

    static  {
        System.out.println("I am in static block");
        b = a * 5;
        // it only runs once , when the first object is create i.e. when the class is loaded for the first time
    }

    public static void main(String[] args) {
        staticBlock obj = new staticBlock();
        System.out.println(obj.a + " " + obj.b);

        staticBlock.b+= 3;
        System.out.println(obj.a + " " + obj.b);

        staticBlock obj2 = new staticBlock();
        System.out.println(obj.a + " " + obj.b);
    }
}
