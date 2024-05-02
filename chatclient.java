import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.PrintWriter;
import java.net.Socket;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.Scanner;

public class ChatClient {
    private static final int BUF_SIZE = 100;
    private static final int NORMAL_SIZE = 20;
    private static String name = "[DEFAULT]";
    private static String servTime;
    private static String servPort;
    private static String clntIp;

    public static void main(String[] args) {
        if (args.length != 3) {
            System.out.println("Usage: java ChatClient <ip> <port> <name>");
            System.exit(1);
        }

        SimpleDateFormat sdf = new SimpleDateFormat("yyyy-MM-dd HH:mm");
        servTime = sdf.format(new Date());

        name = "[" + args[2] + "]";
        clntIp = args[0];
        servPort = args[1];
        int port = Integer.parseInt(servPort);

        try (Socket socket = new Socket(clntIp, port);
             PrintWriter out = new PrintWriter(socket.getOutputStream(), true);
             BufferedReader in = new BufferedReader(new InputStreamReader(socket.getInputStream()));
             Scanner scanner = new Scanner(System.in)) {

            System.out.println(" >> 채팅에 입장하였습니다. ");
            String myInfo = name + " 님이 채팅에 참여하엿습니다.(IP : " + clntIp + ")\n";
            out.println(myInfo);

            Thread sender = new Thread(() -> {
                try {
                    while (true) {
                        String msg = scanner.nextLine();

                        if (msg.equals("!menu")) {
                            menuOptions(scanner, out);
                            continue;
                        } else if (msg.equalsIgnoreCase("q")) {
                            System.exit(0);
                        }

                        out.println(name + " " + msg);
                    }
                } catch (Exception e) {
                    e.printStackTrace();
                }
            });

            Thread receiver = new Thread(() -> {
                try {
                    String msg;
                    while ((msg = in.readLine()) != null) {
                        System.out.println(msg);
                    }
                } catch (IOException e) {
                    e.printStackTrace();
                }
            });

            sender.start();
            receiver.start();

            try {
                sender.join();
                receiver.join();
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        } catch (IOException e) {
            errorHandling("connect() error");
        }
    }

    private static void menuOptions(Scanner scanner, PrintWriter out) {
        System.out.println("\n\t*============ 메뉴 모드 ============\n");
        System.out.println("\t1. 대화명 바꾸기\n");
        System.out.println("\t2. 새로고침 \n\n");
        System.out.println("\t다른 키를 누르면 메뉴 모드를 취소합니다.");
        System.out.println("\n\t=====================================");
        System.out.print("\n\t>> ");

        int select = scanner.nextInt();
        scanner.nextLine(); // consume newline

        switch (select) {
            case 1:
                changeName(scanner);
                break;
            case 2:
                menu();
                break;
            default:
                System.out.println("\tcancel.");
                break;
        }
    }

    private static void changeName(Scanner scanner) {
        System.out.print("\n\t새로운 대화명을 입력하세요. -> ");
        String nameTemp = scanner.nextLine();
        name = "[" + nameTemp + "]";
        System.out.println("\n\t새로고침 완료.\n\n");
    }

    private static void menu() {
        System.out.println(" ========= 클라이언트 정보 =========");
        System.out.println(" 서버 포트     : " + servPort);
        System.out.println(" 클라이언트 IP : " + clntIp);
        System.out.println(" 대화명        : " + name);
        System.out.println(" 서버 시간     : " + servTime);
        System.out.println(" ============== 메뉴 ==============");
        System.out.println(" !menu 입력 시 메뉴 화면으로 이동 ");
        System.out.println(" 1. 대화명 바꾸기 ");
        System.out.println(" 2. 화면 새로고침 ");
        System.out.println(" ==================================");
        System.out.println(" 종료하려면 q & Q 를 입력하세요\n\n");
    }

    private static void errorHandling(String msg) {
        System.err.println(msg);
        System.exit(1);
    }
}


