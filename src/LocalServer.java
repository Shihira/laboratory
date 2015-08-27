import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.channels.SocketChannel;
import java.io.ByteArrayOutputStream;

class LocalHandler extends AsyncServer.ConnectionHandler {
    private SocketChannel channel;
    private ByteArrayOutputStream os;

    @Override
    public void accepted(SocketChannel _channel) {
        channel = _channel;
        os = new ByteArrayOutputStream();

        String strBody = "<h1>Title</h1><p>Body</p>";
        String strHeader =
            "HTTP/1.1 200 OK\r\n" +
            "Content-Length: " + strBody.length() + "\r\n" +
            "Content-Type: text/html\r\n" +
            "\r\n";

        try { os.write((strHeader + strBody).getBytes("UTF-8")); }
        catch(Exception e) { e.printStackTrace(); }
    }

    @Override
    public void closed() { }

    @Override
    public void dataArrived(ByteBuffer bbuf) {
        try {
            System.out.print(new String(bbuf.array(), 0, bbuf.limit(), "UTF-8"));
            if(new String(bbuf.array(), 0, bbuf.limit()).indexOf("\r\n\r\n") > 0)
                channel.write(ByteBuffer.wrap(os.toByteArray()));
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}

public class LocalServer extends AsyncServer {
    public void listen(int port) throws IOException {
        super.listen(port, new ConnectionHandlerFactory() {
            @Override
            public ConnectionHandler createConnectionHandler() {
                return new LocalHandler();
            }
        });
    }
}

