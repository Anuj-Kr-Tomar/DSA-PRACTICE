package staticExample;

public class Main {
    public static void main(String[] args) {
        Human kunal = new Human(22,"kunal",1000,false);
        Human sahil = new Human(20, "sahil", 2000, true);
//        System.out.println(kunal.population);
//        System.out.println(sahil.population);
        // static means you can use that class without creating the object of that class
        //main is what runs the first...
        // in order to run anything that is inside the class you have to create the object of that class i,e. above given class ( public class Main ).
        //so to run psvm it is given static as it is first what runs in program..

       // greeting(); cant't run
    }
    //we know that something is non-static belongs to an object
    void greeting(){
        System.out.println("hello");
        fun();//you can  access static in not static. bcz everything is gonna run in main so defintely an instance will be created for that in the main.
    }

    //this is not dependent on objects
    static void fun(){
       // greeting();
        // you cant use it bcz it requires an instance
        // but the fn you are using it in does not depend on instances.
        Main obj = new Main();
        obj.greeting();
        // you cannot access non-static stuff without referencing their instances in a static context
    }
    // you cannot use this in static bcz this refers to an object but static in independent of object
}
