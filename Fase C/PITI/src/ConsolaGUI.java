import javax.swing.*;
import javax.swing.text.Document;
import java.awt.*;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import com.fazecast.jSerialComm.SerialPort;
import java.awt.event.ItemListener;
import java.awt.event.ItemEvent;
import java.io.IOException;
import java.io.PrintStream;
import java.io.UnsupportedEncodingException;
import java.nio.file.InvalidPathException;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.Locale;


public class ConsolaGUI extends JFrame implements ActionListener, ItemListener {
    private JTextArea textArea;
    private JTextField textField;
    private JComboBox<String> COMs; // Guarda todos as portas serial disponiveís (obtido através de funções)
    private JComboBox<String> Menu_Options; // Guarda todas as opções para o menu (Inato)
    private JButton Start_button = new JButton();
    private String message_from_text_field = "";
    private PrintStream standardOut;
    private F1_2 MetodosDaConsola = new F1_2();
    private SerialPort port_used = null;

    public ConsolaGUI() throws InterruptedException {
        super("OptiSender V1.0");
        JPanel topPanel = new JPanel();

        topPanel.setLayout(new GridLayout(1, 3));

        String[] Portas_para_dropbox = MetodosDaConsola.getCOMS();
        String[] Options = {"Escolha uma opcao","[1] Enviar ficheiro texto","[2] Enviar Imagem","[3] Receber","[4] Enviar mensagem","[5] Receber mensagem"};

        textArea = new JTextArea();
        textArea.setEditable(false);
        // Redireciona-mos o output do sistema para a àrea de texto
        standardOut = System.out;
        PrintStream printStream = new PrintStream(new CustomOutputStream(textArea));
        System.setOut(printStream);
        System.setErr(printStream);
        // Criamos a box de entrada de texto do user
        textField = new JTextField();
        textField.addActionListener(this);

        // Cria os dois dropboxes para facilitar a escolha de algumas opções
        COMs = new JComboBox<>(Portas_para_dropbox);
        Menu_Options = new JComboBox<>(Options);

        //De modo a conseguirmos por mais que um elemento dentro de cada coluna do grid layout, fazemos um nested layout
        JPanel COMsPanel = new JPanel();
        COMsPanel.add(new JLabel("COMs: "));
        COMsPanel.add(COMs);
        COMs.addItemListener(this);

        Start_button.setText("START");
        Start_button.addActionListener(this);

        JPanel OptionsPanel = new JPanel();
        OptionsPanel.add(new JLabel("Opcao: "));
        OptionsPanel.add(Menu_Options);
        Menu_Options.addItemListener(this);

        topPanel.add(COMsPanel);
        topPanel.add(OptionsPanel);
        topPanel.add(Start_button);

        JScrollPane scrollPane = new JScrollPane(textArea);

        JPanel mainPanel = new JPanel(new BorderLayout());
        mainPanel.add(topPanel, BorderLayout.NORTH);
        mainPanel.add(scrollPane, BorderLayout.CENTER);

        setLayout(new BorderLayout());
        add(mainPanel, BorderLayout.CENTER);
        add(textField, BorderLayout.SOUTH);


        setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
        setSize(800, 600);
        setLocationRelativeTo(null);
        setVisible(true);

        textField.requestFocus();

        MetodosDaConsola.Gerar_Menu();

    }

    public void actionPerformed(ActionEvent e) {
        if(e.getSource() == textField) {
            String command = textField.getText();
            message_from_text_field = textField.getText();
            executeCommand(command);
            textField.setText("");
        }
        if(e.getSource() == Start_button && Menu_Options.getSelectedItem() == "[1] Enviar ficheiro texto"){
            boolean flag_path_valido = true;
            Path path_to_use = null;
            try{
                path_to_use = Paths.get(message_from_text_field);
                //System.out.println("Path usado" + path_to_use);
                if(path_to_use.startsWith("")){
                    flag_path_valido = false;
                }
            } catch (InvalidPathException | NullPointerException ex){
                flag_path_valido = false;
            }

            if(flag_path_valido){
                System.out.println("Diretoria valida, vai ser iniciado o processo de envio, alinhe o receptor e transmissor!");
                MetodosDaConsola.Selection_Menu("1",port_used, String.valueOf(path_to_use), (byte) 0b00000010);
            }else{
                System.out.println("Diretoria invalida, vai ser iniciado o processo de envio, alinhe o receptor e transmissor!");
            }
        }else if(e.getSource() == Start_button && Menu_Options.getSelectedItem() == "[2] Enviar Imagem"){
            boolean flag_path_valido = true;
            Path path_to_use = null;
            try{
                path_to_use = Paths.get(message_from_text_field);
                flag_path_valido = true;
            } catch (InvalidPathException | NullPointerException ex){
                flag_path_valido = false;
            }
            if(flag_path_valido){
                System.out.println("Diretoria valida, vai ser iniciado o processo de envio, alinhe o receptor e transmissor!");
                MetodosDaConsola.Selection_Menu("2",port_used, String.valueOf(path_to_use), (byte) 0b00000011);
            }else{
                System.out.println("Diretoria invalida, vai ser iniciado o processo de envio, alinhe o receptor e transmissor!");
            }
        }else if(e.getSource() == Start_button && Menu_Options.getSelectedItem() == "[3] Receber"){
            boolean flag_path_valido = true;
            Path path_to_use = null;
            try{
                path_to_use = Paths.get(message_from_text_field);
                //System.out.println("Path usado" + path_to_use);
                if(path_to_use.startsWith("")){
                    flag_path_valido = false;
                }
            } catch (InvalidPathException | NullPointerException ex){
                flag_path_valido = false;
            }

            if(flag_path_valido){
                System.out.println("Diretoria valida, vai ser iniciado o processo de envio, alinhe o receptor e transmissor!");
                MetodosDaConsola.Selection_Menu("3",port_used, String.valueOf(path_to_use), (byte) 0b00000000);
            }else{
                System.out.println("Diretoria invalida, vai ser iniciado o processo de envio, alinhe o receptor e transmissor!");
            }

        }else if(e.getSource() == Start_button && Menu_Options.getSelectedItem() == "[4] Enviar mensagem"){
            MetodosDaConsola.Selection_Menu("4",port_used, message_from_text_field, (byte) 0b00000100);
        }else if(e.getSource() == Start_button && Menu_Options.getSelectedItem() == "[5] Receber mensagem"){
            MetodosDaConsola.Selection_Menu("5",port_used, message_from_text_field, (byte) 0b00000000);
        }
    }

    private void executeCommand(String command) {
        // Handle the command here
        System.out.println("> " + command);
    }

    private void executeCommandWithDropdown(String dropdownValue) {
        // Handle the dropdown value here
        System.out.println("Dropdown value: " + dropdownValue);
    }

    public static void main(String[] args) {
        SwingUtilities.invokeLater(() -> {
            try {
                new ConsolaGUI();
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        });
    }
    public void itemStateChanged(ItemEvent e){
        if(e.getSource() == COMs && e.getStateChange() == ItemEvent.SELECTED && !e.getItem().toString().contains("Escolha")) {
            String dropdownValue = COMs.getSelectedItem().toString();
            executeCommandWithDropdown(dropdownValue);
            port_used = MetodosDaConsola.Connect_Port(dropdownValue);
            if(port_used != null){
                executeCommand("Conectado com sucesso " + port_used);
            } else{
                executeCommand("Falha na conecexao ao Port");
            }
        }
        if(e.getSource() == COMs && e.getStateChange() == ItemEvent.DESELECTED && !e.getItem().toString().contains("Escolha")){
            String dropdownValue = COMs.getSelectedItem().toString();
            executeCommandWithDropdown(dropdownValue);
            String mensagem_recb = MetodosDaConsola.Fechar_Port(port_used);
            executeCommand(mensagem_recb);
        }
    }

}


class CustomOutputStream extends java.io.OutputStream {
    private JTextArea textArea;

    public CustomOutputStream(JTextArea textArea) {
        this.textArea = textArea;
    }

    public void write(int b) {
        textArea.append(String.valueOf((char) b));
        textArea.setCaretPosition(textArea.getDocument().getLength());
    }
}