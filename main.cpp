#include <iostream> //Standart giriş ve çıkış işlemlerini sağlayan fonksiyonları içerir. 
#include <vector> //Dinamik bir dizi yapısı içerir.
#include <format> //Metinleri biçimlendirmek için bu kütüphaneyi kullanırız.
#include "Header.h" //Kodun başka bir yerinde tanımlanmış olan fonksiyonların ve değişkenlerin bildirimlerini içerir.
#include <TlHelp32.h>   //Windows işletim sistemi için bir kütüphanedir.

// #include Kodları sayesinde kütüphaneleri çağırıyoruz.

DWORD ProcessIdAl(const char* UygulamaIsim) {

	DWORD ProcessId = 0;
	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hSnap != INVALID_HANDLE_VALUE)
	{
		PROCESSENTRY32 ProcessEntry;
		ProcessEntry.dwSize = sizeof(ProcessEntry);

		if (Process32First(hSnap, &ProcessEntry)) {
			do
			{

				if (!_strcmpi((ProcessEntry.szExeFile), UygulamaIsim))
				{
					ProcessId = ProcessEntry.th32ProcessID;
					break;
				}
			} while (Process32Next(hSnap, &ProcessEntry));
		}
	}
	CloseHandle(hSnap);
	return ProcessId;
}

//Yukarıdaki kod belirli bir uygulama adı ile ilgili işlem kimliğini elde etmek için kullanılır.  "Görev yöneticisinin ayrıntılar bölümünde "PID" olarak geçer."


uintptr_t PointerZinciriOku(HANDLE hProc, DWORD64 ptr, std::vector<unsigned int> offsets)
{
	uintptr_t addr = ptr;
	for (unsigned int i = 0; i < offsets.size(); ++i)
	{
		std::cout << "Zincir address " << i + 1 << " : " << std::hex << addr << "\n";
		
		bool Okundumu = ReadProcessMemory(hProc, (BYTE*)addr, &addr, sizeof(DWORD64), 0); 
		std::cout << "Okundumu : " << Okundumu << "\n";
		addr += offsets[i];
	}
	return addr;
}

//Yukarıdaki kod belirtilen oyun üzerinde "Belirtilen Oyun Pro Soccer Online" belirtilen bir işaretçi zinciri ve ofsetler kullanarak bir bellek adresi okumak için kullanılır.

#ifdef _WIN64
static constexpr const auto PEB_OFFSET = 0x60;
#else
static constexpr const auto PEB_OFFSET = 0x30;
#endif

//Yukarıdaki kod 64-bit ve 32-bit ofset değerini belirlemek için bir koşul ifadesi içerir. "64-BİT = (0x60)"  - "32-BİT = (0x30)"

static constexpr const auto OyunIsim = "ProSoccerOnline-Win64-Shipping.exe";

//Yukarıdaki kod "Pro Soccer Online" oyunundaki belirli bir işlemi işaret etmek için kullanılır.

int main() {
	

	DWORD ProcessID = ProcessIdAl("ProSoccerOnline-Win64-Shipping.exe");

	//Yukarıdaki kod "Pro Soccer Online" adlı oyunun işlem kimliğini almak için ProcessIdAl fonksiyonunu kullanır

	std::cout << "Oyun ProcessID : " << std::hex << ProcessIdAl("ProSoccerOnline-Win64-Shipping.exe") << "\n";

	//Yukarıdaki kod "Pro Soccer Online" adlı oyunun işlem kimliğini ekrana yazdırır, hileyi çalıştırdığınızda "Oyun ProcessID'nin" sağına baktığınızda işlem kimliğini görebilirsiniz.

	if (!ProcessID) {
		std::cout << "Oyun ProcessID bulunamadi !"  "\n"; 
		return 0;
	}

	//Hile Eğer oyunun işlem kimliğini bulamaz ise yukarıdaki yazı ekrana çıkar.


	HANDLE OyunHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, ProcessID);

	//Bu kod kısa gözükse bile açıklaması biraz uzun, kısacası bellek alanlarına ve diğer kaynaklarına tam erişim sağlamak için bu kodu kullanıyoruz.

	if (!OyunHandle) {
		std::cout << "Oyun HANDLE alinamadi !"  "\n"; 
		return 0;
	}

	//Yukarıdaki kod fonksiyonun değere eşit olup olmadığını kontrol eder, eğer eşit değilse "Oyun HANDLE alinamadi !" yazısı çıkar.

	DWORD Size = 0;
	PROCESS_BASIC_INFORMATION pbi;
	auto result = NtQueryInformationProc(OyunHandle, ProcessBasicInformation, &pbi, sizeof(PROCESS_BASIC_INFORMATION), &Size);
	PPEB PEB = pbi.PebBaseAddress;
	DWORD64 HedefAdress = ((DWORD64)PEB + 0x10);
	DWORD64 BaseAdress;
	ReadProcessMemory(OyunHandle, (void*)(HedefAdress), &BaseAdress, sizeof(DWORD64), 0);

	//Yukarıdaki kod hedef işlem bellek alanında hedef adrese erişmek için kullanılır.

	DWORD64 BaseAdress2 = (DWORD64)(GetModuleHandle("Console Application.exe")); 
	std::cout << "Oyun Base : " << std::hex << BaseAdress << "\n";
	std::cout << "Oyun PEB : " << std::hex << (DWORD64)PEB << "\n"; 
	
	//Yukarıdaki kod "Pro Soccer Online" adlı oyunun bellek adresini bulmak için kullanılır, sonrasında kendi uygulamamızın bellek adresi alınır en sonunda ekrana yazdırılır.

	if (!BaseAdress) {
		std::cout << "Modul base adresi bulunamadi !"  "\n"; 
		return 0;
	}

	//Eğer oyunun bellek adresi bulunmadıysa yukarıdaki kod sayesinde ekrana "Modul base adresi bunulamadi !" yazısı çıkar.

	std::cout << "Modul adresi : " << std::hex << BaseAdress << "\n";

	//Yukarıdaki kod saklanan bellek adresinin değerini ekrana yazdırır.

	uintptr_t EnAltAdres = (uintptr_t)BaseAdress + 0x46846E8; 

	//Yukarıdaki kod bellek bölgesinin konumunu belirlemek için kullanılır.


	std::cout << "Son Hata :" << std::hex << GetLastError() << "\n";

	//Yukarıdaki kod en son gerçekleşen hata kodunu ekrana yazdırmak için kullanılır.

	auto SonucAdres = PointerZinciriOku(OyunHandle, EnAltAdres, { 0x8 , 0x20 , 0x260 , 0x1A0 , 0x60 , 0x1F8 });
	std::cout << "Son Adress :" << std::hex << SonucAdres << "\n"; 
	float Fov = 0;
	float IstedigimizFov = 140;
	ReadProcessMemory(OyunHandle, (void*)SonucAdres, &Fov, sizeof(float), 0);
	std::cout << std::format("Fov : {:.5} \n", Fov);

	//Yukarıdaki kod'da pointer zincirini belirtiriz, istediğimiz fov değişkeninide 140 olarak atarız.

	while (true)
	{
		WriteProcessMemory(OyunHandle, (void*)SonucAdres, &IstedigimizFov, sizeof(float), 0); 
	}
	//Yukarıdaki kod'dada kısacası oyundaki 120 olan fovumuzu, değişkende kaç belirttiyseniz o yaparsınız. "biz 140 belirttik, 134'üncü satırda görebilirsiniz"
	
	return 0;  //En sonda ise bu kodla çıkışımızı yapıyoruz.
}



//Kod'da yardımcı olduğu içi "hernos" adlı kişiye burdan teşekkürlerimi sunuyorum, kod'da hatalarım oldu hatalarımı bana göstererek yardımcı oldu tekrardan teşekkürler.
