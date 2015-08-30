import java.io.IOException;
import java.net.InetSocketAddress;
import java.nio.ByteBuffer;
import java.nio.channels.SelectionKey;
import java.nio.channels.ServerSocketChannel;
import java.nio.channels.SocketChannel;
import java.io.ByteArrayOutputStream;

final class LocalServerHandler implements AsyncService.AsyncReadHandler {
    private AsyncService service = null;
    private SocketChannel channel = null;
    private ByteArrayOutputStream os = null;

    public LocalServerHandler(SocketChannel channel_, AsyncService service_) {
        channel = channel_;
        service = service_;

        try {
            os = new ByteArrayOutputStream();

            String strBody = "<h1>Title</h1><p>Body</p>";
            String strHeader =
                "HTTP/1.1 200 OK\r\n" +
                "Content-Length: " + strBody.length() + "\r\n" +
                "Content-Type: text/html\r\n" +
                "\r\n";

            os.write((strHeader + strBody).getBytes("UTF-8"));
        } catch(IOException e) {
            e.printStackTrace();
        }
    }

    private class Helper implements AsyncService.AsyncConnectHandler,
            AsyncService.AsyncReadHandler {

        private ByteArrayOutputStream os;
        private SocketChannel forwardChannel;

        public Helper(SocketChannel channel_) {
            try {
                forwardChannel = channel_;
                os = new ByteArrayOutputStream();

                String strHeader = "GET / HTTP/1.1\r\n" +
                    "Host: www.example.com\r\n" +
                    "User-Agent: Mozilla/5.0\r\n" +
                    "Accept: */*\r\n" +
                    "Connection: Keep-Alive\r\n" +
                    "\r\n";

                os.write(strHeader.getBytes("UTF-8"));
            } catch(IOException e) {
                e.printStackTrace();
            }
        }

        @Override
        public void established() {
            try {
                System.out.println(channel.getRemoteAddress().toString() + " Established");
                forwardChannel.write(ByteBuffer.wrap(os.toByteArray()));
            } catch(IOException e) {
                e.printStackTrace();
            }
        }

        @Override
        public void dataArrived(ByteBuffer bbuf) {
            try {
                System.out.print(new String(bbuf.array(), 0, bbuf.limit(), "UTF-8"));

                channel.write(bbuf);
            } catch(IOException e) {
                e.printStackTrace();
            }
        }

        @Override
        public void closing() {
            try {
                System.out.println(channel.getRemoteAddress().toString() + " Closed");
            } catch (Exception e) {
                e.printStackTrace();
            }
        }

        @Override
        public int validOps() {
            return SelectionKey.OP_CONNECT | SelectionKey.OP_READ;
        }
    }
    
    @Override
    public void closing() {
        try {
            System.out.println(channel.getRemoteAddress().toString() + " Closed");
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    @Override
    public void dataArrived(ByteBuffer bbuf) {
        try {
            System.out.print(new String(bbuf.array(), 0, bbuf.limit(), "UTF-8"));

            if(new String(bbuf.array(), 0, bbuf.limit()).indexOf("\r\n\r\n") > 0) {
                SocketChannel forward = SocketChannel.open();
                service.registerChannel(forward, new Helper(forward));

                forward.connect(new InetSocketAddress("www.example.com", 80));
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    @Override
    public int validOps() { return SelectionKey.OP_READ; }
}

public class LocalServer implements AsyncService.AsyncAcceptHandler {

    private AsyncService service = null;
    private ServerSocketChannel listenChannel = null;

    protected LocalServer(AsyncService service_) throws IOException {
        service = service_;
        listenChannel = ServerSocketChannel.open();
    }

    public void listen(int port) throws IOException {
        service.registerChannel(listenChannel, this);
        listenChannel.bind(new InetSocketAddress(port));
    }

    @Override
    public void accepted(SocketChannel channel) {
        try {
            System.out.println(channel.getRemoteAddress().toString() + " Accepted");

            LocalServerHandler handler = new LocalServerHandler(channel, service);
            service.registerChannel(channel, handler);
        } catch(IOException e) {
            e.printStackTrace();
        }
    }

    @Override
    public int validOps() { return SelectionKey.OP_ACCEPT; }

}

