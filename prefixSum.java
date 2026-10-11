import java.net.InetAddress;
import java.net.UnknownHostException;
import java.util.Scanner;

public class A8 {

    // Removes http://, https:// and any path from a URL
    private static String cleanHost(String input) {
        input = input.trim();

        if (input.startsWith("http://")) {
            input = input.substring(7);
        } else if (input.startsWith("https://")) {
            input = input.substring(8);
        }

        int slashIndex = input.indexOf('/');
        if (slashIndex != -1) {
            input = input.substring(0, slashIndex);
        }

        return input;
    }

    // Forward DNS lookup
    private static void forwardLookup(String input) {
        String hostname = cleanHost(input);

        try {
            InetAddress[] addresses = InetAddress.getAllByName(hostname);

            System.out.println("\nHostname: " + hostname);
            System.out.println("IP Addresses:");

            for (InetAddress address : addresses) {
                System.out.println("  " + address.getHostAddress());
            }

            System.out.println("Total IP addresses found: " + addresses.length);

        } catch (UnknownHostException e) {
            System.out.println("\nDNS Lookup Error: Unable to resolve hostname.");
        }
    }

    // Reverse DNS lookup
    private static void reverseLookup(String ipAddress) {
        ipAddress = ipAddress.trim();

        try {
            InetAddress address = InetAddress.getByName(ipAddress);
            String hostname = address.getHostName();

            System.out.println("\nIP Address: " + ipAddress);

            if (hostname.equals(ipAddress)) {
                System.out.println("Hostname: No reverse DNS (PTR) record found.");
            } else {
                System.out.println("Hostname: " + hostname);
            }

        } catch (UnknownHostException e) {
            System.out.println("\nDNS Lookup Error: Invalid IP address or reverse lookup failed.");
        }
    }

    public static void main(String[] args) {

        Scanner scanner = new Scanner(System.in);
        int choice;

        do {
            System.out.println("\n=================================");
            System.out.println("       DNS LOOKUP PROGRAM");
            System.out.println("=================================");
            System.out.println("1. Hostname/URL to IP Address");
            System.out.println("2. IP Address to Hostname");
            System.out.println("3. Exit");
            System.out.print("Enter your choice: ");

            try {
                choice = Integer.parseInt(scanner.nextLine());

                switch (choice) {

                    case 1:
                        System.out.print("\nEnter hostname or URL: ");
                        String hostname = scanner.nextLine();
                        forwardLookup(hostname);
                        break;

                    case 2:
                        System.out.print("\nEnter IP address: ");
                        String ipAddress = scanner.nextLine();
                        reverseLookup(ipAddress);
                        break;

                    case 3:
                        System.out.println("\nExiting DNS Lookup Program...");
                        break;

                    default:
                        System.out.println("\nInvalid choice. Please enter 1, 2 or 3.");
                }

            } catch (NumberFormatException e) {
                System.out.println("\nInvalid input. Please enter a number.");
                choice = 0;
            }

        } while (choice != 3);

        scanner.close();
    }
}
