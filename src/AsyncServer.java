import java.io.IOException;
import java.net.InetSocketAddress;
import java.nio.ByteBuffer;
import java.nio.channels.SelectionKey;
import java.nio.channels.Selector;
import java.nio.channels.ServerSocketChannel;
import java.nio.channels.SocketChannel;
import java.util.HashMap;

public class AsyncServer extends Object {
    protected static abstract class ConnectionHandlerFactory {
        abstract public ConnectionHandler createConnectionHandler();
    }

    protected static abstract class ConnectionHandler {
        public void accepted(SocketChannel channel) { }
        public abstract void dataArrived(ByteBuffer bbuf);
        public void closed() { }
    }

    protected Selector selector;
    protected ServerSocketChannel serverChannel;

    protected HashMap<SocketChannel, ConnectionHandler> connectionPool;

    public AsyncServer() {
        connectionPool = new HashMap<SocketChannel, ConnectionHandler>();

        try {
            selector = Selector.open();
            serverChannel = ServerSocketChannel.open();

            serverChannel.configureBlocking(false);
            serverChannel.register(selector, SelectionKey.OP_ACCEPT);
        } catch(IOException e) {
            e.printStackTrace();
        }
    }

    protected void listen(int port, ConnectionHandlerFactory factory)
        throws IOException {
        serverChannel.bind(new InetSocketAddress(port));

        while(true) {
            selector.selectedKeys().clear();
            selector.select(1000); // TODO: make it an option

            for(SelectionKey key : selector.selectedKeys()) {

                if(key.channel() instanceof ServerSocketChannel) {
                    ServerSocketChannel channel = (ServerSocketChannel) key.channel();

                    if(key.isAcceptable()) {
                        SocketChannel clientChannel = channel.accept();
                        ConnectionHandler handler = factory.createConnectionHandler();

                        clientChannel.configureBlocking(false);
                        clientChannel.register(selector, SelectionKey.OP_READ);
                        handler.accepted(clientChannel);

                        connectionPool.put(clientChannel, handler);
                    }
                }

                if(key.channel() instanceof SocketChannel) {
                    SocketChannel channel = (SocketChannel) key.channel();
                    ConnectionHandler handler = connectionPool.get(channel);

                    if(key.isReadable()) {
                        ByteBuffer bbuf = ByteBuffer.allocate(64);

                        channel.read(bbuf);
                        if(bbuf.position() > 0) {
                            bbuf.flip();
                            handler.dataArrived(bbuf);
                        } else { // closed
                            key.cancel();
                            channel.close();
                            handler.closed();

                            connectionPool.remove(channel);
                        }
                    }
                }

            }

        }
    }
}

