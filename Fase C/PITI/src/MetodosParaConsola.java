import com.fazecast.jSerialComm.SerialPort;

public interface MetodosParaConsola {
   String[] getCOMS() throws InterruptedException;
   SerialPort Connect_Port(String ps_name);
   String Fechar_Port(SerialPort ps);
   void Gerar_Menu();
   void Selection_Menu(String s,SerialPort ps, String path,byte tipo);
}
