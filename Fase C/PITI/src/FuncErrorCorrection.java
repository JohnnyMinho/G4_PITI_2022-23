public interface FuncErrorCorrection {
    byte[] codificarRS(byte[] mensagem);
    byte[] decodificarRS(byte[] mensagem);
}
