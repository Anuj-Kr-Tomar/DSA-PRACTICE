package OOPS;

public class intro {
    public static void main(String[] args) {

        Student kunal = new Student(13,"ANUJ", 88.4f);
        Student rahul = new Student(15,"prem",99.5f);
        kunal.greetings();
        System.out.println(kunal.rno);
        System.out.println(rahul.name);

        Student random = new Student(kunal);
        System.out.println(random.name);

    }
}
class Student {
    int rno;
    String name;
    float marks;

    void greetings(){
        System.out.println("hello my name is " + this.name);
    }

    void changeName(String newName) {
        this.name = newName;
    }

    Student (Student other) {
        this.name = other.name;
        this.rno = other.rno;
        this.marks = other.marks;
    }
    Student(int rno, String name, float marks) {
        this.rno = rno;
        this.name = name;
        this.marks = marks;
    }
    

}
