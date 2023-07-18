package Code;
import java.io.DataOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.ArrayList;
import java.util.List;

import com.fazecast.jSerialComm.SerialPort;

import Enum.EnumStatoUart;
import Enum.EnumTipoPacchettoUart;
import Struct.StructIdentificativo;
import Struct.StructOperazione;
import Struct.StructValore;

public class SerialPortArduino {
	private static SerialPortArduino serialPortArduino = new SerialPortArduino();
	private SerialPort serialPort;
	private SmartHomeBot bot;

	private List<String> nomi_sensori = new ArrayList<>();

	List<StructOperazione> operazioni = new ArrayList<>();

	List<StructIdentificativo> identificativi = new ArrayList<>();

	List<StructValore> valori_sensori = new ArrayList<>();

	String nome_sensore;

	char sof = 'a';
	char eof = 'r';
	char padding = 's';

	List<Character> receive_byte = new ArrayList<>();		
	List<String> valori = new ArrayList<>();

	EnumStatoUart.stato_uart stato_attuale = EnumStatoUart.stato_uart.IN_ATTESA_SOF;

	private void initList() {
		operazioni.add(new StructOperazione(EnumTipoPacchettoUart.tipo_pacchetto_uart.GET_INFO, 'b'));
		operazioni.add(new StructOperazione(EnumTipoPacchettoUart.tipo_pacchetto_uart.SEND_VALUE, 'c'));
		operazioni.add(new StructOperazione(EnumTipoPacchettoUart.tipo_pacchetto_uart.SET_PARAMETER, 'd'));

		nomi_sensori.add("fontana");
		nomi_sensori.add("luce_esterna");
		nomi_sensori.add("temperatura");
		nomi_sensori.add("configurazione_fontana");
		nomi_sensori.add("configurazione_temperatura");
		nomi_sensori.add("sistema_condizionamento");

		identificativi.add(new StructIdentificativo(nomi_sensori.get(0), 'f'));
		identificativi.add(new StructIdentificativo(nomi_sensori.get(1), 'h'));
		identificativi.add(new StructIdentificativo(nomi_sensori.get(2), 'k'));
		identificativi.add(new StructIdentificativo(nomi_sensori.get(3), 'm'));
		identificativi.add(new StructIdentificativo(nomi_sensori.get(4), 'p'));
		identificativi.add(new StructIdentificativo(nomi_sensori.get(5), 'q'));

		valori_sensori.add(new StructValore(nomi_sensori.get(0), List.of('0', '1', '2', '3'), List.of("spenta", "accesa", "acqua insufficiente", "accendi")));
		valori_sensori.add(new StructValore(nomi_sensori.get(1), List.of('0', '1'), List.of("spenta", "accesa")));
		valori_sensori.add(new StructValore(nomi_sensori.get(3), List.of('0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '-'), 
				List.of("0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "-")));
		valori_sensori.add(new StructValore(nomi_sensori.get(3), List.of('0', '1', '2', '3', '4', '5', '6', '7', '8', '9'), 
				List.of("0", "1", "2", "3", "4", "5", "6", "7", "8", "9")));
		valori_sensori.add(new StructValore(nomi_sensori.get(4), List.of('0', '1', '2', '3', '4', '5', '6', '7', '8', '9'), 
				List.of("0", "1", "2", "3", "4", "5", "6", "7", "8", "9")));
		valori_sensori.add(new StructValore(nomi_sensori.get(5), List.of('0', '1'), List.of("spento", "acceso")));
	}

	private void initSerial() {
		serialPort = SerialPort.getCommPort("xxxxxxxx");
		final InputStream inputStream = serialPort.getInputStream();
		serialPort.setComPortParameters(9600, 8, 1, 0);
		serialPort.openPort();
		serialPort.flushIOBuffers();
		Thread readThread = new Thread(new Runnable() {	
			public void run() {
				// TODO Auto-generated method stub
				while(true) {
					try {
						if(inputStream.available() > 0) {
							char read = (char) inputStream.read();
							if (processByte(read)) {
								receive_byte.add(read);
								//è arrivato il valore 1? è arrivato il valore 2? è arrivato il valore 3?
								if (stato_attuale == EnumStatoUart.stato_uart.IN_ATTESA_VALORE_2 ||
										stato_attuale == EnumStatoUart.stato_uart.IN_ATTESA_VALORE_3 ||
										stato_attuale == EnumStatoUart.stato_uart.IN_ATTESA_EOF) {
									if (read != padding) {
										valori.add(valori_sensori.get(nomi_sensori.indexOf(nome_sensore)).getValoriTradotti(read));								
									}
								} else {
									//ho ricevuto tutto il pacchetto
									if (stato_attuale == EnumStatoUart.stato_uart.IN_ATTESA_SOF) {
										String messaggio = nome_sensore + ": ";
										for (String v : valori) {
											messaggio += v;
										}
										bot.sendNotification(messaggio);
										valori.clear();
									}
								}
							}
						}
					} catch (IOException e) {
						// TODO Auto-generated catch block
						e.printStackTrace();
					}
				}
			}
		});
		readThread.start();
	}

	private Boolean processByte(char b) {
		Boolean ok = false;
		switch (stato_attuale) {
		case IN_ATTESA_SOF:
			if (b == sof) {
				stato_attuale = EnumStatoUart.stato_uart.IN_ATTESA_OPERAZIONE;
				ok = true;
			}
			break;
		case IN_ATTESA_OPERAZIONE:
			for (StructOperazione tmp : operazioni) {
				if (tmp.valore == b) {
					stato_attuale = EnumStatoUart.stato_uart.IN_ATTESA_IDENTIFICATIVO;
					ok = true;
				}
			}
			break;
		case IN_ATTESA_IDENTIFICATIVO:
			for (StructIdentificativo tmp : identificativi) {
				if (tmp.valore == b) {
					nome_sensore = tmp.nome_sensore;
					stato_attuale = EnumStatoUart.stato_uart.IN_ATTESA_VALORE_1;
					ok = true;
				}
			}
			break;
		case IN_ATTESA_VALORE_1:
			try {
				for (String nome : nomi_sensori) {
					if (nome.equals(nome_sensore)) {
						for (char tmp : valori_sensori.get(nomi_sensori.indexOf(nome)).valori) {
							if (b == tmp) {
								ok = true;
								throw new RuntimeException();
							}
						}
					}
				}
			} catch (Exception e) {
				ok = true;
				stato_attuale = EnumStatoUart.stato_uart.IN_ATTESA_VALORE_2;
			}
			break;
		case IN_ATTESA_VALORE_2:
			//è ammesso padding
			try {
				if (b == padding) {
					ok = true;
					stato_attuale = EnumStatoUart.stato_uart.IN_ATTESA_VALORE_3;
				} else {
					for (String nome : nomi_sensori) {
						if (nome.equals(nome_sensore)) {
							for (char tmp : valori_sensori.get(nomi_sensori.indexOf(nome)).valori) {
								if (b == tmp) {
									ok = true;
									throw new RuntimeException();
								}
							}
						}
					}
				}				
			} catch (Exception e) {
				ok = true;
				stato_attuale = EnumStatoUart.stato_uart.IN_ATTESA_VALORE_3;
			}
			break;
		case IN_ATTESA_VALORE_3:
			//è ammesso padding
			try {
				if (b == padding) {
					ok = true;
					stato_attuale = EnumStatoUart.stato_uart.IN_ATTESA_EOF;
				} else {
					for (String nome : nomi_sensori) {
						if (nome.equals(nome_sensore)) {
							for (char tmp : valori_sensori.get(nomi_sensori.indexOf(nome)).valori) {
								if (b == tmp) {
									ok = true;
									throw new RuntimeException();
								}
							}
						}
					}
				}				
			} catch (Exception e) {
				ok = true;
				stato_attuale = EnumStatoUart.stato_uart.IN_ATTESA_EOF;
			}
			break;
		case IN_ATTESA_EOF:
			if (b == eof) {
				stato_attuale = EnumStatoUart.stato_uart.IN_ATTESA_SOF;
				ok = true;
			}
			break;
		default:
			break;
		}
		return ok;
	}

	private SerialPortArduino() {
		initList();
		initSerial();
	}
	
	public static SerialPortArduino getInstance() {
		return serialPortArduino;
	}
	
	public void setBotInstance(SmartHomeBot bot) {
		this.bot = bot;
	}
	
	public void requestTelegram() {
		OutputStream outputStream = serialPort.getOutputStream();
		DataOutputStream dataOutputStream = new DataOutputStream(outputStream);
		String msg = String.valueOf(sof) + String.valueOf('b') + String.valueOf(padding) + String.valueOf(padding) + String.valueOf(padding) + String.valueOf(padding) + String.valueOf(eof);
		try {
			dataOutputStream.write(msg.getBytes());
			dataOutputStream.flush();
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
	}
}
