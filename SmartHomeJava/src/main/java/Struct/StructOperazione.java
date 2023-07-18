package Struct;

import Enum.EnumTipoPacchettoUart;

public class StructOperazione {
	public EnumTipoPacchettoUart.tipo_pacchetto_uart tipo_pacchetto_uart;
	public char valore;
	
	public StructOperazione(EnumTipoPacchettoUart.tipo_pacchetto_uart tipo_pacchetto_uart, char valore) {
		this.tipo_pacchetto_uart = tipo_pacchetto_uart;
		this.valore = valore;
	}
}
