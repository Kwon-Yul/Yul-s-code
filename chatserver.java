import java.io.*;
import java.net.*;
import java.util.*;
import java.util.concurrent.locks.*;

public class ChatServer {
    private static final int BUF_SIZE = 100;
    private static final int MAX_CLNT = 5;
    private static int clntCnt = 0;
    private static Socket[] clntSocks = new Socket[MAX_CLNT];
    private static Lock lock = new ReentrantLock();

    public static void main(String[] args) {
        if (args.length != 1) {
            System.out.println("Usage: java ChatServer <port>");
            System.exit(1);
        }

        menu(args[0]);

        try (ServerSocket serverSocket = new ServerSocket(Integer.parseInt(args[0]))) {
            System.out.println("Server started. Listening on Port " + args[0]);

            while (true) {
                if (clntCnt >= MAX_CLNT) {
                    System.out.println("No more connections can be accepted. Maximum number of clients exceeded.");
                    continue;
                }

                Socket clntSock = serverSocket.accept();
                lock.lock();
                try {
                    clntSocks[clntCnt++] = clntSock;
                } finally {
                    lock.unlock();
                }

                Thread t = new Thread(() -> handleClient(clntSock));
                t.start();

                System.out.println("Connected client IP(Time): " + clntSock.getInetAddress().getHostAddress() + " (" + new Date() + ")");
                System.out.println("Number of connections: (" + clntCnt + "/5)");
            }
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    private static void handleClient(Socket clntSock) {
        try {
            InputStream input = clntSock.getInputStream();
            OutputStream output = clntSock.getOutputStream();
            byte[] buf = new byte[BUF_SIZE];
            int len;

            while ((len = input.read(buf)) != -1) {
                sendMsg(buf, len);
            }
        } catch (IOException e) {
            System.out.println("Client " + clntSock + " disconnected.");
        } finally {
            lock.lock();
            try {
                for (int i = 0; i < clntCnt; i++) {
                    if (clntSock.equals(clntSocks[i])) {
                        while (i < clntCnt - 1) {
                            clntSocks[i] = clntSocks[i + 1];
                            i++;
                        }
                        clntCnt--;
                        break;
                    }
                }
            } finally {
                lock.unlock();
                try {
                    clntSock.close();
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
        }
    }

    private static void sendMsg(byte[] msg, int len) {
        lock.lock();
        try {
            for (int i = 0; i < clntCnt; i++) {
                clntSocks[i].getOutputStream().write(msg, 0, len);
            }
        } catch (IOException e) {
            e.printStackTrace();
        } finally {
            lock.unlock();
        }
    }

    private static void menu(String port) {
        System.out.println("========= Chat Server Status =========");
        System.out.println("Server Port     : " + port);
        System.out.println("Server Status   : " + serverState(clntCnt));
        System.out.println("Max Clients     : " + MAX_CLNT);
        System.out.println("========== Connection Log ==========\n");
    }

    private static String serverState(int count) {
        if (count < 5)
            return "Connection Stable";
        else
            return "Connection Unstable";
    }
}


