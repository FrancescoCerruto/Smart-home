package Code;
import org.telegram.telegrambots.bots.TelegramLongPollingBot;
import org.telegram.telegrambots.meta.api.methods.send.SendMessage;
import org.telegram.telegrambots.meta.api.objects.Update;
import org.telegram.telegrambots.meta.exceptions.TelegramApiException;

public class SmartHomeBot extends TelegramLongPollingBot {
	
	private String chatId = "xxxxxxxxx";
	SerialPortSt serialPortSt;
	SerialPortArduino serialPortArduino;
	private String command = "/get_info_smart_home";
	
	public SmartHomeBot() {
		serialPortSt = SerialPortSt.getInstance();
		serialPortSt.setBotInstance(this);
		serialPortArduino = SerialPortArduino.getInstance();
		serialPortArduino.setBotInstance(this);
	}
	
	public void onUpdateReceived(Update update) {
		if (!update.getMessage().getChatId().toString().equals(chatId)) {
			SendMessage response = new SendMessage();
			response.setChatId(update.getMessage().getChatId().toString());
			response.setText("Non sei autorizzato ad utilizzare questo bot");
			try {
				execute(response);
			} catch (TelegramApiException e) {
				e.printStackTrace();
			}
		} else {
			String request = update.getMessage().getText();
			if (request.equals(command)) {
				serialPortSt.requestTelegram();
				serialPortArduino.requestTelegram();
			} else {
				sendNotification("Comando non riconosciuto");
			}
		}
	}

	public String getBotUsername() {
		// TODO Auto-generated method stub
		return "xxxxxxxxxxxxxxxxxx";
	}

	public String getBotToken() {
		// TODO Auto-generated method stub
		return "xxxxxxxxxxxxxxxxxxx";
	}
	
	public void sendNotification(String messaggio) {
		SendMessage response = new SendMessage();
		response.setChatId(chatId);
		response.setText(messaggio);
		try {
			execute(response);
		} catch (TelegramApiException e) {
			e.printStackTrace();
		}
	}
}
