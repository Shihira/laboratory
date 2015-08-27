public class Main {
    public static void main(String[] args) {
        try {
            LocalServer server = new LocalServer();
            server.listen(9999);
        } catch(Exception e) {
            e.printStackTrace();
        }
    }
}
