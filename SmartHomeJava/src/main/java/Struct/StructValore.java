package Struct;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

public class StructValore {
	public String nome_sensore;
	public List<Character> valori = new ArrayList<>();
	private Map<Character, String> valori_tradotti = new HashMap<>();
	
	public StructValore(String nome_sensore, List<Character> valori, List<String> valori_tradotti) {
		this.nome_sensore = nome_sensore;
		for (Character v : valori) {
			this.valori.add(v);
			this.valori_tradotti.put(v, valori_tradotti.get(this.valori.indexOf(v)));
		}
	}
	
	public String getValoriTradotti(Character b) {
		return valori_tradotti.get(b);
	}
}
