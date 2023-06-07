import javax.swing.*;
import java.awt.*;
import java.io.File;
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Paths;

public class VisualizadordeFicheiros {

    public void openFileViewer(String filePath, String file_type) {
            File file = new File(filePath);

            try {
                if (file_type == "Imagem") {
                    // Display image file
                    ImageIcon imageIcon = new ImageIcon(filePath);
                    JFrame frame = new JFrame("Image Viewer");
                    frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
                    frame.setSize(imageIcon.getIconWidth() + 20, imageIcon.getIconHeight() + 40);
                    frame.setLocationRelativeTo(null);
                    JLabel imageLabel = new JLabel(imageIcon);
                    frame.getContentPane().add(imageLabel);
                    frame.setVisible(true);
                } else if(file_type == "Texto") {
                    // Display text file
                    String fileContent = new String(Files.readAllBytes(Paths.get(filePath)));
                    JFrame frame = new JFrame("Text Viewer");
                    frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
                    frame.setSize(400, 300);
                    frame.setLocationRelativeTo(null);
                    JTextArea textArea = new JTextArea();
                    textArea.setEditable(false);
                    textArea.setText(fileContent);
                    frame.getContentPane().add(new JScrollPane(textArea));
                    frame.setVisible(true);
                } else{
                    String fileContent = "Não consigo apresentar este ficheiro";
                    JFrame frame = new JFrame("Text Viewer");
                    frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
                    frame.setSize(400, 300);
                    frame.setLocationRelativeTo(null);
                    JTextArea textArea = new JTextArea();
                    textArea.setEditable(false);
                    textArea.setText(fileContent);
                    frame.getContentPane().add(new JScrollPane(textArea));
                    frame.setVisible(true);
                }
            } catch (IOException e) {
                e.printStackTrace();
            }
    }
    public void main(String[] args) {
        openFileViewer("/home/johnnyminho/Documents/PITI/Fase A/Aviao.jpg","Imagem");
    }
}
