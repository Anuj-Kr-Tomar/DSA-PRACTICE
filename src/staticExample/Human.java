package staticExample;

public class Human {
    int age;
    String name;
    int salary;
    boolean married;
    static long population;// static variable are independent of object you can access them even though you have not created any object

    public Human(int age, String name, int salary, boolean married) {
        this.age = age;
        this.name = name;
        this.salary = salary;
        this.married = married;
//      this.population += 1;
        Human.population += 1;


    }

}
