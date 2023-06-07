import com.fazecast.jSerialComm.SerialPort;
import static java.lang.Thread.sleep;
import static net.fec.openrq.parameters.ParameterChecker.maxAllowedDataLength;

import javax.imageio.ImageIO;

import java.awt.*;
import java.awt.image.BufferedImage;
import java.io.*;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Iterator;

public class F1_2 implements MetodosParaConsola{

    private MétodosCorrErros CorrErros= new MétodosCorrErros(); //Para aceder aos métoos da correção de erros
    private VisualizadordeFicheiros VerFile = new VisualizadordeFicheiros();

    public static final long MAX_DATA_LEN = 32; //O Máximo de bytes por bloco é 32 bytes, os bytes de paridade que acompanham os mesmos na mensagem, não fazem parte deste limite

    public void main(String[] args) throws IOException, InterruptedException
    {

//https://fazecast.github.io/jSerialComm/

    }

    //Parte Funções de geração de dados para o GUI
        public void Gerar_Menu(){
            System.out.println("*******************************************");
            System.out.println("* Transmissao otica sob infravermelhos *");
            System.out.println("* Trabalho realizado por: Grupo 4           *");
            System.out.println("* Curso: MIETI                                         *");
            System.out.println("*******************************************");
            System.out.println("Por uma questao de simplicidade, agora conseguimos escolher tanto as portas serie\n tal como as opcoes de funcionamento a partir desta interface grafica");
            System.out.println("Escreva o path do ficheiro a enviar");
            System.out.println("Para utilizar alguma das opcoes tem de fazer o seguinte\n Envio - Digite e introduza a diretoria do ficheiro a enviar. \n Receber - Digite e introduz a diretoria onde quer guardar o ficheiro");
        }

        public void Selection_Menu(String op, SerialPort ps, String path, byte tipo){
        try {
            System.out.print("port"+ps);
            if(ps != null){
            switch (op) {
                case "1" -> enviar(ps,path,tipo); //Método de envio
                case "2" -> enviar(ps,path,tipo); //Método de envio para imagem
                case "3" -> receber(ps,path);//Método para receber txt
                case "4" -> enviarConversa(ps,path); //reutilizamos o path como uma maneira de não ter de enviar mais dados para a função
                case "5" -> receberconversa(ps); //reutilizamos o path como uma maneira de não ter de enviar mais dados para a função
            }}else{
                System.out.print("Nao ha nenhum port selecionado, nao foi realizada qualquer op");
            }
        }catch (IOException e){
            System.out.print("Ocorreu um erro durante a execução a sua opção");
        }
        }

    //Parte Ports
    public SerialPort Connect_Port(String ps_name){
        SerialPort ps = SerialPort.getCommPort(ps_name);
        if(ps.openPort()) {
            ps.setComPortParameters(115200, 8, 0, 0);   //Definir a Confg da Porta serie
            return ps;
        }
        else{
            return null;
        }
    }

    public String Fechar_Port(SerialPort ps){
        if(ps == null){
            return "Port fechada não estava disponível";
        }
        if(ps.closePort()) {
            return "Port fechado com sucesso";
        }
        else{
            return "Port não foi fechado com sucesso";
        }
    }

    public String[] getCOMS() throws InterruptedException {
        SerialPort[] p = SerialPort.getCommPorts();
        ArrayList<String> ValoresPorts = new ArrayList<>();
        //String[] ValoresPorts = new String[p.length];
        if (p.length == 0) {
            System.out.println("Portas indisponiveis");
            sleep(2000);
            System.exit(0);
        }
        int i = 0;
        int k = 0;
            for (SerialPort ps : p) {     //Seleção da porta série a usar
                if(ps.getSystemPortName().contains("USB") || ps.getSystemPortName().contains("COM") ) {
                    ValoresPorts.add(ps.getSystemPortName());
                    k++;
                }
                i++;
            }
            String[] array_envio = new String[k+1];
            array_envio[0] = "Escolha um dos Ports";
            for(int n = 0; n<k; n++ ){
                array_envio[n+1] = ValoresPorts.get(n);
            }
        return array_envio;
    }

    // Parte específica receptor
    public void receber(SerialPort port, String Path_to_Store) throws IOException {
        ArrayList<Byte> todos_os_dados = new ArrayList<>();
        String slash_to_use = "\\\\";
        if(!Path_to_Store.contains(slash_to_use)){
            slash_to_use = "/";
        }
        //InputStream inputStream = port.getInputStream();
        boolean flag_hello_received = false; //Nós têmos de esperar pela primeira mensagem de hello para sabermos o nome e extensão de ficheiro.
        boolean stop_message_recb = false;
        String Nome_ficheiro = "";
        String Extensao_ficheiro = "";
        boolean ficheiro_text = false;
        boolean ficheiro_imagem = false;
        boolean mensagem_chat = false;
        boolean can_write_to_file = false;
        String filename = Path_to_Store + slash_to_use; //Pequeno bug nesta zona, é preciso por o
        byte[] cab = new byte[5];
        //Como a trama que suporta o envio das informações, a Hello, têm um cabeçalho diferente, precisamos de verificar sempre se esta à parte das restantes.
        while(!stop_message_recb) {
            /*
            port.readBytes(cab,cab.length);
            System.out.println(Arrays.toString(cab));
            for(int i=0; i<5;i++){
                cab[i]=0;
            }
            stop_message_recb=true;*/
            if (port.bytesAvailable() == 3 && !flag_hello_received) {
                byte[] cab_hello = new byte[3];
                port.readBytes(cab_hello, 3);
                System.out.println(Arrays.toString(cab_hello));
                flag_hello_received = true;
                int quantidade_dados = 1 + (int) cab_hello[2];
                    byte[] dados = new byte[quantidade_dados];
                    System.out.println(("Valor:" + quantidade_dados));
                    while (port.bytesAvailable() < quantidade_dados) {
                        //Espera pelo envio de todos os bytes do cabecalho da trama do tipo hello
                    }
                    port.readBytes(dados, quantidade_dados);
                    byte[] bytes_tipo = Arrays.copyOfRange(dados, 0, 6);
                    System.out.println(Arrays.toString(bytes_tipo));
                    byte[] bytes_nome = Arrays.copyOfRange(dados, bytes_tipo.length, quantidade_dados - 1);
                    System.out.println(Arrays.toString(bytes_nome));
                    String resultado_decodificar_tipo = new String(CorrErros.decodificarRS(bytes_tipo));
                    String resultado_decodificar_nome = new String(CorrErros.decodificarRS(bytes_nome));
                    System.out.println(resultado_decodificar_nome);
                    System.out.println(resultado_decodificar_tipo);
                    StringBuilder output = new StringBuilder();
                    StringBuilder output2 = new StringBuilder();
                    for (int i = 0; i < resultado_decodificar_tipo.length(); i++) {
                        char c = resultado_decodificar_tipo.charAt(i);
                        if (c <= 122 && c >= 65) {
                            output.append(c);
                        }
                    }
                    String extensao = output.toString();
                    for (int i = resultado_decodificar_tipo.length(); i < resultado_decodificar_nome.length(); i++) {
                        char c = resultado_decodificar_nome.charAt(i);
                        if (c <= 122 && c >= 65) {
                            output2.append(c);
                        }
                    }
                    String nome = output2.toString();
                    //System.out.println(output.toString());

                    Extensao_ficheiro = extensao;
                    Nome_ficheiro = nome;
                    System.out.println("Extensao:" + resultado_decodificar_tipo);
                    System.out.println("Nome:" + resultado_decodificar_nome);
                    filename = filename + resultado_decodificar_nome +"." + resultado_decodificar_tipo;
                    System.out.println("Nome ficheiro = " + filename);
            }
            if (flag_hello_received) {
                //Têmos de esperar que a mensagem de stop seja recebida para sabermos que deixamos de ter dados para ler e que podemos guardar os dados no ficheiro
                while (!stop_message_recb) {
                    int counter_debug = 0;
                    long wait_timer = System.currentTimeMillis(); //Como a trama stop só tẽm 3 bytes, ela fica aqui presa.
                    while(port.bytesAvailable() < 5)  {
                        //Espera pelo envio do último byte
                        if(System.currentTimeMillis()-wait_timer >= 1000){
                            can_write_to_file = true;
                            break;
                        }
                    }
                    port.readBytes(cab, 5);

                    System.out.println(cab[1]);

                    if (cab[0] == 0b00000010) { //Se tivermos recebido a start flag, a qual indica que começamos a receber dados de uma trama de x tipo.
                        int quantidade_dados2 = (int) cab[3];
                        switch (cab[1]) {
                            case 0b00000000: //HELLO QUE INDICA O TIPO DE DADOS, EXTENSÃO E NOME DO FICHEIRO
                                //Como os dados já foram tratados, este ponto é simplesmente ignorado
                                break;
                            case 0b00000010: //Caso o ficheiro seja um txt ou algum tipo de ficheiro de leitura
                                System.out.println("Texto");
                                ficheiro_text = true;
                                break;
                            case 0b00000011: //Caso o ficheiro seja uma imagem
                                System.out.println("Imagem");
                                ficheiro_imagem = true;
                                break;
                            case 0b00000100: //Caso seja uma mensagem de chat
                                System.out.println("Chat");
                                mensagem_chat = true;
                                break;
                            case 0b00000101:
                                System.out.println("Stop");
                                stop_message_recb = true;
                                break;
                        }
                        if (!stop_message_recb && cab[1]!= 0b00000000) {
                            byte[] dados = new byte[quantidade_dados2]; //A quantidade de dados incluí os bytes de paridade

                            while (port.bytesAvailable() < quantidade_dados2) {
                                //Espera que os dados estejam todos no buffer do port para serem lidos
                            }
                            //System.out.println("Teste");
                            port.readBytes(dados, quantidade_dados2);
                            System.out.println("Dados:"+Arrays.toString(dados));
                            byte[] cauda = new byte[2];
                            while (port.bytesAvailable() < 2) {
                                //Espera pelo envio da cauda
                            }
                            //System.out.println("Teste2");
                            port.readBytes(cauda, 2);
                            if (cauda[1] == 0b00000011) { //Se recebermos a stop flag, guardamos os dados desta trama e indicamos
                                System.out.println("StopFlag");
                                byte[] resultado_decodificar = CorrErros.decodificarRS(dados); //Decodificamos os dados da trama recebida
                                System.out.println(Arrays.toString(resultado_decodificar));
                                todos_os_dados.addAll(Arrays.asList(toObject(resultado_decodificar))); //Foi indicado o fim da trama, logo os dados são adicionados
                                if (can_write_to_file) {
                                    if (ficheiro_text) {
                                        System.out.println("Hello");
                                        StringBuilder sb = new StringBuilder();
                                        for (byte b : todos_os_dados) {
                                            sb.append((char) b);
                                        }
                                        String para_file = sb.toString();
                                        FileWriter fileWriter = new FileWriter(filename);
                                        fileWriter.write(para_file);
                                        fileWriter.close();
                                    }
                                    if (ficheiro_imagem) {
                                        System.out.println("Hello");
                                        byte[] byteArray = new byte[todos_os_dados.size()];
                                        for (int i = 0; i < todos_os_dados.size(); i++) {
                                            byteArray[i] = todos_os_dados.get(i);
                                        }
                                        ByteArrayInputStream bais = new ByteArrayInputStream(byteArray);

                                        // Converte os bytes recebidos em uma imagem
                                        BufferedImage image = ImageIO.read(bais);

                                        // A imagem recebido é colocada na diretoria com o nome e extensão indicados
                                        File outputFile = new File(filename);
                                        ImageIO.write(image, Extensao_ficheiro, outputFile);
                                    }
                                    stop_message_recb = true;
                                }
                            }
                        }
                        else {
                            if (ficheiro_text) {
                                System.out.println("Hello");
                                StringBuilder sb = new StringBuilder();
                                for (byte b : todos_os_dados) {
                                    sb.append((char) b);
                                }
                                String para_file = sb.toString();
                                FileWriter fileWriter = new FileWriter(filename);
                                fileWriter.write(para_file);
                                fileWriter.close();
                            }
                            if (ficheiro_imagem) { // Recebe os bytes mas não consegue converter
                                byte[] byteArray = new byte[todos_os_dados.size()];
                                for (int i = 0; i < todos_os_dados.size(); i++) {
                                        byteArray[i] = todos_os_dados.get(i);
                                }
                                System.out.println("Nome file"+filename);
                                ByteArrayInputStream bais = new ByteArrayInputStream(byteArray);
                                BufferedImage buffered_image= ImageIO.read(new File("/home/johnnyminho/Documents/PITI/pinch.png"));
                                ByteArrayOutputStream output_stream= new ByteArrayOutputStream();
                                ImageIO.write(buffered_image, "png", output_stream);
                                byte [] byte_array2 = output_stream.toByteArray();
                                System.out.println(Arrays.toString(byte_array2));
                                System.out.println(Arrays.toString(bais.readAllBytes()));
                                System.out.println("hello");
                                // Converte os bytes recebidos em uma imagem
                                BufferedImage image = ImageIO.read(bais);
                                // A imagem recebido é colocada na diretoria com o nome e extensão indicados

                                File outputFile = new File(filename);
                                //System.out.println("Hello");
                                ImageIO.write(image, Extensao_ficheiro, outputFile);
                            }
                            if (mensagem_chat) {
                                System.out.println(todos_os_dados);
                            }
                        }
                    }


                /*
                byte[] fix = new byte[10];
                while (port.bytesAvailable() < 10) {
                }
                port.readBytes(fix, 10);
                */
                }
            }
        }
    }

    // Fim parte específica receptor

    //Funções de apoio emissor

    public byte[] formularCabeçalho(int tamanho_dados, int n_envio, byte tipo, int n_fluxo){

        byte[] cabecalho = new byte[5];
        int tamanho_total = 0;
        cabecalho[0] = 0b00000010;
        System.out.println("Tipo:" + tipo);
        cabecalho[1] = tipo;
        cabecalho[2] = (byte) tamanho_dados;
        cabecalho[3] = (byte) n_envio;
        cabecalho[4] = (byte) n_fluxo;
        //cabecalho[5] = '\0';
        return cabecalho;
    }
    public byte[] trama_inicio(int tamanho_file, String tipo_file, String nome){
        byte[] aux = tipo_file.getBytes();
        byte[] tipo_file_codificado = CorrErros.codificarRS(aux);
        byte[] aux2 = nome.getBytes();
        byte[] nome_file_codificado = CorrErros.codificarRS(aux2);
        byte[] trama = new byte[4+nome_file_codificado.length+tipo_file_codificado.length];
        trama[0]= 0b00000010;
        trama[1]= 0b00000000;//hello
        trama[2] = (byte) (nome_file_codificado.length+tipo_file_codificado.length);
        System.arraycopy(tipo_file_codificado, 0, trama, 3, tipo_file_codificado.length);
        System.arraycopy(nome_file_codificado, 0, trama, 3+tipo_file_codificado.length, nome_file_codificado.length);
        trama[3+nome_file_codificado.length+tipo_file_codificado.length]= 0b00000011;
        //trama[4+nome_file_codificado.length+tipo_file_codificado.length]='\0';

        return trama;
    }

    //Fim funções de apoio emissor

    //Início para a função principal do emissor

    //Método para enviar o ficheiro de texto
    public void enviar(SerialPort port, String path,byte tipo) throws IOException
    {
        int b=32;
        //C:\\Users\\Marco\\Desktop\\PITI\\PITI\\teste.txt
        String slash_to_use = "\\\\";
        if(!path.contains(slash_to_use)){
            slash_to_use = "/";
        }
        System.out.print(path);
        String[] partes = path.split(slash_to_use);
        //System.out.print(Arrays.toString(partes));
        if(partes.length == 1){
            partes = path.split(slash_to_use);
        }
        String[] partes2 = partes[partes.length-1].split("\\.");
        //System.out.println(Arrays.toString(partes2));
        byte[] bytesFromFile = Files.readAllBytes(Paths.get(path));//Converte o ficheiro de texto para um array de bytes
        //System.out.println("Teste:" + Arrays.toString(bytesFromFile));
        int n_bytes_file = bytesFromFile.length;
        //System.out.println("Tamanho_file:"+n_bytes_file);
        int i=0,j=0;
        byte[] flag =new byte[1];
        boolean first_Time = true;
        boolean can_data_trans = true;
        flag[0]=0;
        int cont=0;
        byte[] trama_inicial=trama_inicio(bytesFromFile.length, partes2[1], partes2[0]);
        boolean data_to_send = true;
        boolean stop_sent = false;
        while(!stop_sent) {
            while(i==0 && !first_Time){
                port.readBytes(flag,1 );
                //System.out.println(flag[0]);
                if(flag[0]==1){
                    flag[0] = 0;
                    i=1;
                }
            }
            if(first_Time){
                first_Time = false;
                String size_trama = String.valueOf(trama_inicial.length);
                byte[] to_send_size = size_trama.getBytes();
                if(to_send_size.length == 1){
                    to_send_size = new byte[]{'0', to_send_size[0]};
                }
                //System.out.println("Tamanho hello:"+ to_send_size[0] + to_send_size[1]);
                System.out.println(Arrays.toString(trama_inicial));
                port.writeBytes(to_send_size,to_send_size.length);
                //System.out.println("TESTE: " + to_send_size);
                try {
                    Thread.sleep(250);
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
                port.writeBytes(trama_inicial,trama_inicial.length);
            }
            if(i==1){
                if(n_bytes_file<b){
                    b = n_bytes_file; //Assim evitamos a exception OutOfBounds
                    data_to_send = false;
                }else{
                    n_bytes_file = n_bytes_file-b;
                }
                    if(can_data_trans){
                        byte[] subArray = Arrays.copyOfRange(bytesFromFile, j, j+b); //Tiramos os bytes do ficheiro em blocos do tamanho j+b
                        //System.out.print("subArray:" + Arrays.toString(subArray));
                        byte[] bytes_codificados = CorrErros.codificarRS(subArray);
                        System.out.println(Arrays.toString(bytes_codificados));
                        j=j+b;
                        System.out.println("Resultado:" );
                        byte[] cab=formularCabeçalho(cont, bytes_codificados.length, tipo, 0); //Formular o cabeçalho
                        byte[] subArray2 = new byte[cab.length+bytes_codificados.length+2];
                        //System.out.println();
                        System.arraycopy(cab, 0, subArray2,0,cab.length); //Substitui o Arrays.copyofrange visto que este simplesmente cria um array novo que ignora a definition anterior do espaço declarado para o pacote
                        //System.out.println(subArray2.length);
                        //System.out.println(subArray.length);
                        System.out.println(Arrays.toString(cab));
                        System.arraycopy(bytes_codificados, 0, subArray2,cab.length,bytes_codificados.length);
                        subArray2[cab.length+bytes_codificados.length+1] = 0b00000011;
                        System.out.println(Arrays.toString(subArray2));
                        String size_trama = String.valueOf(subArray2.length);
                        byte[] to_send_size = size_trama.getBytes();
                        if(to_send_size.length == 1){
                            to_send_size = new byte[]{'0', to_send_size[0]};
                        }
                        //System.out.println(Arrays.toString(to_send_size));
                        port.writeBytes(to_send_size,to_send_size.length);
                        try {
                            Thread.sleep(250);
                        } catch (InterruptedException e) {
                            e.printStackTrace();
                        }
                        port.writeBytes(subArray2,subArray2.length);  //Envia para a porta série o array de bytes
                        cont++;
                        i=0;
                        if(!data_to_send){
                            //System.out.println("data_to_send_false");
                            can_data_trans = false;
                        }
                    }else{
                        System.out.println("STOP");
                        //Por uma questão de simplicidae, o stop é declarado desta maneira para o receptor ficando o memso responsável por formular e enviar a mesma
                        byte[] to_send_size = new byte[]{'9', '9'};
                        //System.out.println(Arrays.toString(to_send_size));
                        port.writeBytes(to_send_size,to_send_size.length);
                        stop_sent = true;
                    }
            }

        }
    }

    public void receberconversa(SerialPort port) throws IOException{
        byte[] byte_lido = new byte[1];
        while(port.bytesAvailable() < 1){
            while(port.bytesAvailable()>1) {
                port.readBytes(byte_lido, 1);
                System.out.println(Arrays.toString(byte_lido));
            }
        }
    }

    public void enviarConversa(SerialPort port,String from_box) throws IOException
    {
        int b=32;
        byte[] from_input = from_box.getBytes();
        byte[] byte_codificados = CorrErros.codificarRS(from_input);
        //byte[] bytesFromFile = Files.readAllBytes(Paths.get("C:\\Users\\Marco\\Desktop\\PITI\\PITI\\teste.txt"));//Converte o ficheiro de texto para um array de bytes
        //ArrayDataEncoder encoder_temp = getEncoder(bytesFromFile,)
        byte[] cab = formularCabeçalho(byte_codificados.length,0,(byte) 0b00000100,0);
        byte[] packet = new byte[5+byte_codificados.length+1];
        System.arraycopy(cab,0,packet,0,cab.length);
        System.arraycopy(byte_codificados,0,packet,cab.length,byte_codificados.length);
        packet[5+byte_codificados.length] = (byte) 0b00000011;
        System.out.println("Pacote:"+Arrays.toString(packet));
        byte[] fake_hello = {0b00000010,0b00000000 , 0};//hello
        port.writeBytes(fake_hello,fake_hello.length);
        System.out.println(Arrays.toString(fake_hello));
        try {
            Thread.sleep(250);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
        port.writeBytes(packet,packet.length);
    }
    private static Byte[] toObject(byte[] byteArray) {
        Byte[] byteObjects = new Byte[byteArray.length];
        for (int i = 0; i < byteArray.length; i++) {
            byteObjects[i] = byteArray[i];
        }
        return byteObjects;
    }
    //Fim parte principal emissão

}