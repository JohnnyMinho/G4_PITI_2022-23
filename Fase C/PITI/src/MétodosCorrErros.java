import com.backblaze.erasure.ReedSolomon;

import java.util.ArrayList;
import java.util.Arrays;

public class MétodosCorrErros  implements FuncErrorCorrection{
    //Correção de erros com recurso ao protocolo FEC Reed-Solomon através da biblioteca JavaReedSolomon da BackBlaze
    /*Através dos parâmetros que usamos, ou seja, dividir a mensagem em 4 blocos de n bytes com 2 blocos do mesmo tamanho dedicados  à deteção e correção de erros para cálculo da paridade
     * da mensagem recebida e partida em 4 blocos, sendo a paridade cáculada de modo a ser guardada nos 2 blocos de paridade.
     * Realizando cálculos a nível teórico, sabemos que o overhead vai sempre ser de 1,5, isto sem a inclusão dos bytes de cabeçalho. Logo por cada mensagem enviada para o ESP32, 1/3 da mensagem vai ser bytes para o controlo e deteção de erros
     * Esta classe é dívidida em duas partes, codificação e descodificação.
     * Na codificação, apenas é necessário enviar o bloco da mensagem a ser codificado, sendo que este devolve um array de duas dimensões onde está contido cada bloco e os conteúdos do mesmo
     * Passando os conteúdos para um único array, estes podem ser encapsulados com o cabeçalho e enviados para o receptor
     * No receptor, este irá usar a parte do descodificador, este também têm de receber apenas o bloco recebido, isto sem o cabeçalho e stop flag
     * Estando estas pontos cumpridos, a mensagem é descodificada na sua totalidade, incluindo os bytes de padding.
     * Estes bytes são removidos na mesma função através da procura por bytes 0x00 no ArrayList constituido por toda a mensagem descodificada.
     * A quantidade de erros possíveis de corrigir com a técnica de codificação RS é cálculada da seguinte maneira, N = bytes de dados , P = bytes de paridade
     * Te = N-P / 2 , logo numa mensagem de 32 bytes é posssível corrigir 8 erros */

    public byte[] decodificarRS(byte[] mensagem){
        final int DATA_SHARDS = 4;
        final int PARITY_SHARDS = 2;
        final int TOTAL_SHARDS = 6;
        ArrayList<Integer> Positions_with_padding = new ArrayList<>();
        int tamanho_mensagem = mensagem.length;
        final int BYTES_IN_INT = 4;
        // Read in any of the shards that are present.
        // (There should be checking here to make sure the input
        // shards are the same size, but there isn't.)
        final byte [] [] shards = new byte [TOTAL_SHARDS] [];
        final boolean [] shardPresent = new boolean [TOTAL_SHARDS];
        int shardSize = 0;
        int shardCount = 0;
        ReedSolomon reedSolomon = new ReedSolomon(DATA_SHARDS, PARITY_SHARDS);

        for (int i = 0; i < TOTAL_SHARDS; i++) {
            int count = (int) (tamanho_mensagem / (3.0 / 2.0));
            shardSize = (count)/DATA_SHARDS;
            shards[i] = new byte [shardSize];
            shardPresent[i] = true;
            shardCount += 1;
            System.out.println("SharedSize:" + shardSize);
            System.arraycopy(mensagem, i * shardSize, shards[i], 0, shardSize);
            System.out.println("Current Shard: " + Arrays.toString(shards[i]));
            System.out.println("Read " + "Message");
        }

        // We need at least DATA_SHARDS to be able to reconstruct the file.
        if (shardCount < DATA_SHARDS) {
            byte[] error = new byte[1];
            error[0] = '*';
            System.out.println("Not enough shards present");
            return error;
        }

        int [] confirmar_paridade = reedSolomon.isParityCorrect2(shards, 0, shardSize);
        System.out.println((Arrays.toString(confirmar_paridade)));
        int counter = 0;
        int m = 0;
        int n = 0;
        for(int r = 0; r < shards.length; r++){
            for(int c = 0 ; c <shards[r].length;c++){
                if(r == 0){
                    m = 1;
                }else{
                    m = r;
                }
                if(r!= 0 && c == 0){
                    n = c+1;
                }
                System.out.println(m*n);
                if(confirmar_paridade[m*n] != 0){
                    System.out.println("ERROR");
                    shardPresent[r] = false;
                }
            }
        }

        // Make empty buffers for the missing shards.
        for (int i = 0; i < TOTAL_SHARDS; i++) {
            if (!shardPresent[i]) {
                shards[i] = new byte [shardSize];
            }
        }

        // Use Reed-Solomon to fill in the missing shards

        //System.out.println(reedSolomon.isParityCorrect(shards, 0, shardSize));
        //Uma possível maneira de confirmar que não foi por dsync da paridade é refazer os blocos de paridade e verificar se os valores do array devolvido na isparitycorrect2 são todos 0
        //Caso o mesmo aconteça, podemos afirmar que o erro estava nos bytes de paridade e que por isso pode ser ignorado o erro.

        reedSolomon.decodeMissing(shards, shardPresent, 0, shardSize);
        // Combine the data shards into one buffer for convenience.
        // (This is not efficient, but it is convenient.)
        byte [] allBytes = new byte [shardSize * DATA_SHARDS];
        for (int i = 0; i < DATA_SHARDS; i++) {
            System.arraycopy(shards[i], 0, allBytes, shardSize * i, shardSize);
        }
        int max_pad_count = 0;
        ArrayList<Byte> new_bytes_temp = new ArrayList<>();
        for (int i = 0; i < allBytes.length; i++){
            if(allBytes[i] != 0x00){
                new_bytes_temp.add(allBytes[i]);
            }
        }
        byte[] new_allBytes = new byte[new_bytes_temp.size()];
        for (int i = 0; i < new_bytes_temp.size(); i++) {
            new_allBytes[i] = new_bytes_temp.get(i);
        }
        System.out.println(Arrays.toString(new_allBytes));
        return new_allBytes;
    }
    public byte[] codificarRS(byte[] mensagem) {
        final int DATA_SHARDS = 4; //N = 4
        final int PARITY_SHARDS = 2; // K = 2
        final int TOTAL_SHARDS = 6; // N-K = 12 (distância) máximo de erros corrigidos -> (N-K)/2 = 8
        // final int BYTES_IN_INT = 4;

        int conta_bytes = 0;
        for (byte byte_lido : mensagem) {
            if (byte_lido != 0) {
                conta_bytes++;
            }
        }
        //System.out.println("N bytes: " + conta_bytes);
        //Verificamos se têmos uma mensagem com conteúdo
        //Caso não tenha, a função ira returnar um array com um *.
        //Caso A mensagem não tenha bytes suficientes para se usar o esquema pretendido, é lhe adicionado padding com 0
        if (conta_bytes == 0) {
            byte[] error_array = new byte[1];
            error_array[0] = '*';
            System.out.println("A mensagem nao tem conteudo");
            return error_array;
        } else if ((conta_bytes % 4) != 0) {
            //System.out.println("Hello");
            while (conta_bytes % 4 != 0) {
                conta_bytes++;
                mensagem = Arrays.copyOf(mensagem, conta_bytes);
                //System.out.println(Arrays.toString(mensagem));
            }
        }
        System.out.println("Final size: " + conta_bytes);
        byte[] mensagem_used = new byte[conta_bytes]; //Como no Java a definição das variáveis têm de ser indicada fora das condições / loops de modo a ser usada fora dos mesmos. Utilizamos este método
        int contador = 0;
        while (contador != conta_bytes) {
            /*if (mensagem[contador] == 0) {
                mensagem[contador] = '#';
            } else {*/
                mensagem_used[contador] = mensagem[contador];
                System.out.println(mensagem_used[contador]);
            //}
            contador++;
        }

        //O ficheiro não pode avançar o tamanho máximo de uma variável INT (2147483647)
        final int tamanhoMSG = mensagem_used.length;

        final int shardSize = tamanhoMSG / DATA_SHARDS;
        System.out.println("Tamanho:" + shardSize);
        // Create a buffer holding the file size, followed by
        // the contents of the file.

        final int bufferSize = shardSize * DATA_SHARDS;
        final byte[] allBytes = new byte[bufferSize];

        // Make the buffers to hold the shards.
        byte[][] shards = new byte[TOTAL_SHARDS][shardSize];

        // Fill in the data shards
        for (int i = 0; i < DATA_SHARDS; i++) {
            System.arraycopy(mensagem_used, i * shardSize, shards[i], 0, shardSize);
        }

        // Use Reed-Solomon to calculate the parity.
        ReedSolomon reedSolomon = new ReedSolomon(DATA_SHARDS, PARITY_SHARDS);

        reedSolomon.encodeParity(shards, 0, shardSize);
        System.out.println(reedSolomon.isParityCorrect(shards, 0, shardSize));
        System.out.println(Arrays.toString(reedSolomon.isParityCorrect2(shards, 0, shardSize)));
        //System.out.println(Arrays.deepToString(shards));
        //System.out.println(shards.length);
        ArrayList<Byte> resultado_bytes = new ArrayList<Byte>();
        for (int i = 0; i < shards.length; i++) {
            for (int j = 0; j < shards[i].length; j++) {
                //System.out.print(resultado[i][j] + " ");
                resultado_bytes.add(shards[i][j]);
                //System.out.print(resultado_bytes.get(j) + " ");
            }
        }
        // System.out.println(resultado_bytes.size());
        byte[] bytetosend = new byte[resultado_bytes.size()];
        for (int i = 0; i < resultado_bytes.size(); i++) {
            bytetosend[i] = resultado_bytes.get(i);
        }
        return bytetosend;
    }
   /* public void main(String[] args) throws IOException, InterruptedException {


        /*
        String Teste = "Hello";
        byte[] teste_byte = Teste.getBytes(StandardCharsets.US_ASCII);
        //System.out.print(Arrays.toString(teste_byte));
        byte[][] resultado = codificarRS(teste_byte);
        //System.out.println(resultado.length);
        ArrayList<Byte> resultado_bytes = new ArrayList<Byte>();
        for (int i = 0; i < resultado.length; i++) {
            for (int j = 0; j < resultado[i].length; j++) {
                //System.out.print(resultado[i][j] + " ");
                resultado_bytes.add(resultado[i][j]);
                //System.out.print(resultado_bytes[j] + " ");
            }
        }
       // System.out.println(resultado_bytes.size());
        byte[] bytetosend = new byte[resultado_bytes.size()];
        for (int i = 0; i < resultado_bytes.size(); i++) {
            bytetosend[i] = resultado_bytes.get(i);
        }

        byte[] resultado_decoded = decodificarRS(bytetosend);
        for (int i = 0; i < resultado_decoded.length; i++) {
            char traduzido = (char) resultado_decoded[i];
            System.out.print(traduzido);
        }

        // Write out the resulting files.
        /*
            for (int i = 0; i < TOTAL_SHARDS; i++) {
                File outputFile = new File(
                        inputFile.getParentFile(),
                        inputFile.getName() + "." + i);
                OutputStream out = new FileOutputStream(outputFile);
                out.write(shards[i]);
                out.close();
                System.out.println("wrote " + outputFile);
            }

        System.out.println(Arrays.deepToString(resultado));
    }

    public static byte[] decodificarRS(byte[] mensagem){
        final int DATA_SHARDS = 4;
        final int PARITY_SHARDS = 2;
        final int TOTAL_SHARDS = 6;
        ArrayList<Integer> Positions_with_padding = new ArrayList<>();
        int tamanho_mensagem = mensagem.length;
        final int BYTES_IN_INT = 4;
            // Read in any of the shards that are present.
            // (There should be checking here to make sure the input
            // shards are the same size, but there isn't.)
            final byte [] [] shards = new byte [TOTAL_SHARDS] [];
            final boolean [] shardPresent = new boolean [TOTAL_SHARDS];
            int shardSize = 0;
            int shardCount = 0;

            for (int i = 0; i < TOTAL_SHARDS; i++) {
                int count = (int) (tamanho_mensagem / (3.0 / 2.0));
                shardSize = (count)/DATA_SHARDS;
                shards[i] = new byte [shardSize];
                shardPresent[i] = true;
                shardCount += 1;
                System.out.println("SharedSize:" + shardSize);
                System.arraycopy(mensagem, i * shardSize, shards[i], 0, shardSize);
                System.out.println("Current Shard: " + Arrays.toString(shards[i]));
                System.out.println("Read " + "Message");
            }

            // We need at least DATA_SHARDS to be able to reconstruct the file.
            if (shardCount < DATA_SHARDS) {
                byte[] error = new byte[1];
                error[0] = '*';
                System.out.println("Not enough shards present");
                return error;
            }

            // Make empty buffers for the missing shards.
            for (int i = 0; i < TOTAL_SHARDS; i++) {
                if (!shardPresent[i]) {
                    shards[i] = new byte [shardSize];
                }
            }

            // Use Reed-Solomon to fill in the missing shards
            ReedSolomon reedSolomon = new ReedSolomon(DATA_SHARDS, PARITY_SHARDS);
            reedSolomon.decodeMissing(shards, shardPresent, 0, shardSize);

            // Combine the data shards into one buffer for convenience.
            // (This is not efficient, but it is convenient.)
            byte [] allBytes = new byte [shardSize * DATA_SHARDS];
            for (int i = 0; i < DATA_SHARDS; i++) {
                System.arraycopy(shards[i], 0, allBytes, shardSize * i, shardSize);
            }
            int max_pad_count = 0;
            ArrayList<Byte> new_bytes_temp = new ArrayList<>();
            for (int i = 0; i < allBytes.length; i++){
                if(allBytes[i] != 0x00){
                    new_bytes_temp.add(allBytes[i]);
                }
            }
            byte[] new_allBytes = new byte[new_bytes_temp.size()];
            for (int i = 0; i < new_bytes_temp.size(); i++) {
                new_allBytes[i] = new_bytes_temp.get(i);
            }
            System.out.println(Arrays.toString(new_allBytes));
            return new_allBytes;
}
    byte[][] codificarRS(byte[] mensagem) {
        final int DATA_SHARDS = 4; //N = 4
        final int PARITY_SHARDS = 2; // K = 2
        final int TOTAL_SHARDS = 6; // N-K = 12 (distância) máximo de erros corrigidos -> (N-K)/2 = 8
        // final int BYTES_IN_INT = 4;

        int conta_bytes = 0;
        for (byte byte_lido : mensagem) {
            if (byte_lido != 0) {
                conta_bytes++;
            }
        }
        System.out.println("N bytes: " + conta_bytes);
        //Verificamos se têmos uma mensagem com conteúdo
        //Caso não tenha, a função ira returnar um array com um *.
        //Caso A mensagem não tenha bytes suficientes para se usar o esquema pretendido, é lhe adicionado padding com '*'
        if (conta_bytes == 0) {
            byte[][] error_array = new byte[1][1];
            error_array[0][0] = '*';
            System.out.println("A mensagem não têm conteúdo");
            return error_array;
        } else if ((conta_bytes % 4) != 0) {
            System.out.println("Hello");
            while (conta_bytes % 4 != 0) {
                conta_bytes++;
                mensagem = Arrays.copyOf(mensagem, conta_bytes);
                //System.out.println(Arrays.toString(mensagem));
            }
        }
        System.out.println("Final size: " + conta_bytes);
        byte[] mensagem_used = new byte[conta_bytes]; //Como no Java a definição das variáveis têm de ser indicada fora das condições / loops de modo a ser usada fora dos mesmos. Utilizamos este método
        int contador = 0;
        while (contador != conta_bytes) {
            if (mensagem[contador] == 0) {
                mensagem[contador] = '#';
            } else {
                mensagem_used[contador] = mensagem[contador];
                //System.out.println(mensagem_used[contador]);
            }
            contador++;
        }

        //O ficheiro não pode avançar o tamanho máximo de uma variável INT (2147483647)
        final int tamanhoMSG = mensagem_used.length;

        final int shardSize = tamanhoMSG / DATA_SHARDS;
        System.out.println("Tamanho:" + shardSize);
        // Create a buffer holding the file size, followed by
        // the contents of the file.

        final int bufferSize = shardSize * DATA_SHARDS;
        final byte[] allBytes = new byte[bufferSize];

        // Make the buffers to hold the shards.
        byte[][] shards = new byte[TOTAL_SHARDS][shardSize];

        // Fill in the data shards
        for (int i = 0; i < DATA_SHARDS; i++) {
            System.arraycopy(mensagem_used, i * shardSize, shards[i], 0, shardSize);

        }

        // Use Reed-Solomon to calculate the parity.
        ReedSolomon reedSolomon = new ReedSolomon(DATA_SHARDS, PARITY_SHARDS);
        reedSolomon.encodeParity(shards, 0, shardSize);
        //System.out.println(Arrays.deepToString(shards));
        //System.out.println(shards.length);
        return shards;
    }
}*/

    }

