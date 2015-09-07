public class Main {
    public static void main(String[] args) {
        try {
            AsyncService service = new AsyncService();
            LocalServer server = new LocalServer(service);

            server.listen(9999);

            service.run();
        } catch(Exception e) {
            e.printStackTrace();
        }
    }
}
