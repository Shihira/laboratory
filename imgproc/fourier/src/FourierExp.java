import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import java.awt.image.BufferedImage;

interface ImageGenerator {
    public BufferedImage loadImage();
}

class LocalImage implements ImageGenerator {
    String filename;
    Component comp;

    public LocalImage(String fn, Component c) {
        filename = fn;
        comp = c;
    }

    public BufferedImage loadImage() {
        Image img = Toolkit.getDefaultToolkit().createImage(filename);
        MediaTracker mt = new MediaTracker(comp);
        mt.addImage(img, 1);
        try {
            mt.waitForID(1);
        } catch(Exception e) {
            System.err.println(e.getMessage());
        }

        BufferedImage oimg = new BufferedImage(img.getWidth(null), img.getHeight(null), BufferedImage.TYPE_INT_BGR);

        Graphics g = oimg.getGraphics();
        g.drawImage(img, 0, 0, null);

        return oimg;
    }
};

class SineWaveImage implements ImageGenerator {
    public BufferedImage loadImage() {
        BufferedImage oimg = new BufferedImage(256, 256, BufferedImage.TYPE_INT_BGR);
        for(int j = 0; j < 256; j++)
            for(int i = 0; i < 256; i++) {
                double v = Math.cos(5 * i / 255.0 * 2 * Math.PI);
                v *= 0.5;
                v += 0.5;
                v *= 255;
                int int_v = (int)v;
                int_v = int_v > 255 ? 255 : int_v < 0 ? 0 : int_v;
                oimg.setRGB(i, j, (int_v << 16) | (int_v << 8) | int_v);
            }
        return oimg;
    }
};

class SquareWaveImage implements ImageGenerator {
    public BufferedImage loadImage() {
        BufferedImage oimg = new BufferedImage(256, 256, BufferedImage.TYPE_INT_BGR);
        for(int j = 0; j < 256; j++)
            for(int i = 0; i < 256; i++) {
                if(j % 256 < 128)
                    oimg.setRGB(i, j, 0);
                else
                    oimg.setRGB(i, j, 0xffffff);
            }
        return oimg;
    }
}

public class FourierExp extends JFrame {
    static final long serialVersionUID = 1L;

    public BufferedImage originImg;
    public BufferedImage fourierImg;
    double cosFourierResult[][];
    double sinFourierResult[][];

    public JLabel picViewer;
    public JPanel switchButtonsPanel;
    public JPanel loadButtonsPanel;
    public JPanel imagePanel;
    public JButton toFourierImage;
    public JButton toOriginImage;

    void updateImage(Image img) {
        ImageIcon imgIcon = new ImageIcon(img);
        picViewer.setIcon(imgIcon);
        pack();
    }

    JButton getLoadImageButton(String title, final ImageGenerator gen) {
        JButton btn = new JButton(title);
        btn.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                originImg = gen.loadImage();
                updateImage(originImg);
            }
        });
        return btn;
    }

    public FourierExp() {
        picViewer = new JLabel();
        switchButtonsPanel = new JPanel();
        loadButtonsPanel = new JPanel();
        imagePanel = new JPanel();
        toFourierImage = new JButton("To Fourier");
        toOriginImage = new JButton("To Origin");

        setLayout(new BoxLayout(getContentPane(), BoxLayout.Y_AXIS));
        add(imagePanel);
        add(switchButtonsPanel);
        add(loadButtonsPanel);
        imagePanel.add(picViewer);

        switchButtonsPanel.add(toFourierImage);
        switchButtonsPanel.add(toOriginImage);
        loadButtonsPanel.add(getLoadImageButton("Lena", new LocalImage("lena.jpg", this)));
        loadButtonsPanel.add(getLoadImageButton("Sine Wave", new SineWaveImage()));
        loadButtonsPanel.add(getLoadImageButton("Square Wave", new SquareWaveImage()));

        addWindowListener(new WindowAdapter() {
            public void windowClosing(WindowEvent e) { System.exit(0); }
        });
        toFourierImage.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) { originToFourier(); }
        });
        toOriginImage.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) { fourierToOrigin(); }
        });

        pack();
    }

    public static void main(String[] args) {
        FourierExp window = new FourierExp();
        window.setVisible(true);
    }

    void fourierToOrigin() {
        int width = cosFourierResult.length;
        int level = width;
        int hlfw = width / 2;
        int hlfl = level / 2;

        double cosFunc[][] = new double[level][level];
        double sinFunc[][] = new double[level][level];

        for(int l = -hlfl; l < hlfl; l += 1) {
            for(int w = 0; w < width; w += 1) {
                double x = l;
                x *= w - hlfw;
                x /= hlfw;
                x *= Math.PI;
                cosFunc[l + hlfl][w] = Math.cos(x);
                sinFunc[l + hlfl][w] = Math.sin(x);
            }
        }

        double cosRowVal[][] = new double[width][width];
        double sinRowVal[][] = new double[width][width];
        for(int c = 0; c < width; c += 1) {
            for(int r = 0; r < width; r += 1) {
                double cosVal = 0;
                double sinVal = 0;
                for(int l = 0; l < level; l += 1) {
                    cosVal += cosFourierResult[c][l] * cosFunc[l][r];
                    cosVal -= sinFourierResult[c][l] * sinFunc[l][r];
                    sinVal += cosFourierResult[c][l] * sinFunc[l][r];
                    sinVal += sinFourierResult[c][l] * cosFunc[l][r];
                }
                cosRowVal[c][r] = cosVal;
                sinRowVal[c][r] = sinVal;
            }
        }

        double cosFinVal[][] = new double[width][width];
        double sinFinVal[][] = new double[width][width];
        for(int r = 0; r < width; r += 1) {
            for(int c = 0; c < width; c += 1) {
                double cosVal = 0;
                double sinVal = 0;
                for(int l = 0; l < level; l += 1) {
                    cosVal += cosRowVal[l][r] * cosFunc[l][c];
                    sinVal += sinRowVal[l][r] * sinFunc[l][c];
                }
                cosFinVal[c][r] = cosVal;
                sinFinVal[c][r] = sinVal;
            }
        }

        originImg = new BufferedImage(width, width, BufferedImage.TYPE_INT_BGR);
        for(int c = 0; c < width; c += 1) {
            for(int r = 0; r < width; r += 1) {
                double cosVal = cosFinVal[c][r];
                double sinVal = sinFinVal[c][r];
                double val = cosVal - sinVal;

                int intv = (int)(val * 256);
                intv = intv > 255 ? 255 : intv < 0 ? 0 : intv;

                originImg.setRGB(c, r, (intv << 16) | (intv << 8) | intv);
            }
        }

        updateImage(originImg);
    }

    void originToFourier() {
        int width = originImg.getWidth();
        int level = width;
        int hlfw = width / 2;
        int hlfl = level / 2;

        double cosFunc[][] = new double[level][level];
        double sinFunc[][] = new double[level][level];

        for(int l = -hlfl; l < hlfl; l += 1) {
            for(int w = 0; w < width; w += 1) {
                double x = l;
                x *= w - hlfw;
                x /= hlfw;
                x *= Math.PI;
                cosFunc[l + hlfl][w] = Math.cos(-x);
                sinFunc[l + hlfl][w] = Math.sin(-x);
            }
        }

        double orgVal[][] = new double[level][level];
        for(int r = 0; r < level; r++) {
            for(int c = 0; c < level; c += 1) {
                int color = originImg.getRGB(c, r);
                orgVal[c][r] =
                    ((color & 0xff) +
                    ((color >> 8) & 0xff) +
                    ((color >> 16) & 0xff)) / 3;
                orgVal[c][r] /= 255.0;
            }
        }

        ////////////////////////////////////////////////////////////////////////

        double cosRowVal[][] = new double[level][level];
        double sinRowVal[][] = new double[level][level];
        for(int r = 0; r < level; r += 1) { // row
            for(int l = 0; l < level; l += 1) {
                double cosVal = 0;
                double sinVal = 0;
                for(int w = 0; w < width; w += 1) {
                    cosVal += cosFunc[l][w] * orgVal[w][r] * (1.0/width);
                    sinVal += sinFunc[l][w] * orgVal[w][r] * (1.0/width);
                }
                cosRowVal[l][r] = cosVal;
                sinRowVal[l][r] = sinVal;
            }
        }

        double cosFinVal[][] = new double[level][level];
        double sinFinVal[][] = new double[level][level];
        for(int c = 0; c < level; c += 1) { // column
            for(int l = 0; l < level; l += 1) {
                double cosVal = 0;
                double sinVal = 0;
                for(int w = 0; w < width; w += 1) {
                    cosVal += cosFunc[l][w] * cosRowVal[c][w] * (1.0/width);
                    cosVal -= sinFunc[l][w] * sinRowVal[c][w] * (1.0/width);
                    sinVal += cosFunc[l][w] * sinRowVal[c][w] * (1.0/width);
                    sinVal += sinFunc[l][w] * cosRowVal[c][w] * (1.0/width);
                }
                cosFinVal[c][l] = cosVal;
                sinFinVal[c][l] = sinVal;
            }
        }

        ////////////////////////////////////////////////////////////////////////

        fourierImg = new BufferedImage(level, level, BufferedImage.TYPE_INT_BGR);

        for(int x = 0; x < level; x++) {
            for(int y = 0; y < level; y++) {
                double cosVal = cosFinVal[x][y];
                double sinVal = sinFinVal[x][y];
                double val = Math.sqrt(cosVal * cosVal + sinVal * sinVal);

                int intv = (int)(val * 255.0 * 10);
                intv = intv > 255 ? 255 : intv < 0 ? 0 : intv;
                fourierImg.setRGB(x, y, (intv << 16) | (intv << 8) | intv);
            }
        }

        cosFourierResult = cosFinVal;
        sinFourierResult = sinFinVal;
        updateImage(fourierImg);
    }
}

