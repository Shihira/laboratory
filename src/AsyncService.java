import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.channels.SelectableChannel;
import java.nio.channels.SelectionKey;
import java.nio.channels.Selector;
import java.nio.channels.ServerSocketChannel;
import java.nio.channels.SocketChannel;

public class AsyncService implements Runnable {

    private static interface AsyncHandler {
        int validOps();
    }

    public static interface AsyncReadHandler extends AsyncHandler {
        void dataArrived(ByteBuffer bbuf);
        void closing();
    }

    public static interface AsyncAcceptHandler extends AsyncHandler {
        void accepted(SocketChannel channel);
    }

    public static interface AsyncConnectHandler extends AsyncHandler {
        void established();
    }

    private Selector selector = null;

    public AsyncService() throws IOException {
        selector = Selector.open();
    }

    public void run() {
        try {
            while(true) {
                selector.selectedKeys().clear();
                selector.select();

                for(SelectionKey key : selector.selectedKeys()) {

                    if(key.isReadable()) {
                        AsyncReadHandler handler = (AsyncReadHandler) key.attachment();
                        SocketChannel channel = (SocketChannel) key.channel();

                        ByteBuffer bbuf = ByteBuffer.allocate(128);
                        channel.read(bbuf);

                        if(bbuf.position() > 0) {
                            bbuf.flip();
                            handler.dataArrived(bbuf);
                        } else {
                            key.cancel();
                            handler.closing();
                            channel.close();
                            continue;
                        }
                    }

                    if(key.isAcceptable()) {
                        AsyncAcceptHandler handler = (AsyncAcceptHandler) key.attachment();
                        ServerSocketChannel channel = (ServerSocketChannel) key.channel();

                        SocketChannel clientChannel = channel.accept();

                        handler.accepted(clientChannel);
                    }

                    if(key.isConnectable()) {
                        AsyncConnectHandler handler = (AsyncConnectHandler) key.attachment();
                        SocketChannel channel = (SocketChannel) key.channel();

                        channel.finishConnect();

                        handler.established();
                    }
                }
            }
        } catch(IOException e) {
            e.printStackTrace();
        }
    }

    void registerChannel(SelectableChannel channel, AsyncHandler handler)
        throws IOException {
        channel.configureBlocking(false);
        channel.register(selector, handler.validOps(), handler);
    }
}
