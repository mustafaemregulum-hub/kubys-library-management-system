#include "depo.h"
#include "dosya_islemleri.h"
#include "faz1_sorgular.h"
#include "varliklar.h"

#include <iostream>
#include <optional>
#include <string>

int main() {
    Depo<std::string, Kitap> kitaplar;
    Depo<int, Uye> uyeler;
    Depo<int, OduncKaydi> oduncler;

    kitaplar.ekle("978-975-1000-01-1", {"978-975-1000-01-1", "Modern C++ Temelleri", "Selin Arslan", 2020, "Programlama", 4});
    kitaplar.ekle("978-975-1000-02-8", {"978-975-1000-02-8", "Algoritma Tasarimi", "Emre Koc", 2019, "Bilgisayar Bilimi", 2});
    kitaplar.ekle("978-975-1000-03-5", {"978-975-1000-03-5", "Nesne Yonelimli Programlama", "Derya Yalcin", 2023, "Programlama", 3});

    uyeler.ekle(2001, {2001, "Mustafa Emre", "Gulum", "555-1201", "2025-02-03"});
    uyeler.ekle(2002, {2002, "Ali", "Bozkurt", "555-1202", "2025-02-10"});

    oduncler.ekle(1, {1, 2001, "978-975-1000-01-1", "2025-03-01", std::nullopt, OduncDurum::Oduncte});
    oduncler.ekle(2, {2, 2002, "978-975-1000-02-8", "2025-02-12", "2025-02-25", OduncDurum::IadeEdildi});
    oduncler.ekle(3, {3, 2001, "978-975-1000-03-5", "2025-01-15", std::nullopt, OduncDurum::Gecikmis});

    std::cout << "Programlama kategorisindeki kitaplar:\n";
    for (const auto& kitap : kubys::sorgu::kategoriye_gore_kitaplari_listele(kitaplar, "Programlama")) {
        std::cout << "  " << kitap << '\n';
    }

    std::cout << "\nGecikmis kayitlar:\n";
    for (const auto& kayit : kubys::sorgu::gecikmis_iade_kayitlarini_bul(oduncler)) {
        std::cout << "  " << kayit << '\n';
    }

    std::cout << "\nYayin yilina gore kitaplar:\n";
    for (const auto& kitap : kubys::sorgu::kitaplari_yayin_yilina_gore_sirala(kitaplar)) {
        std::cout << "  " << kitap << '\n';
    }

    std::cout << "\n2001 no'lu uyenin odunc sayisi: "
              << kubys::sorgu::uyenin_toplam_odunc_sayisini_hesapla(oduncler, 2001)
              << '\n';

    if (const auto enCok = kubys::sorgu::en_cok_odunc_alinan_kitabi_bul(kitaplar, oduncler)) {
        std::cout << "\nEn cok odunc alinan kitap: " << enCok.value() << '\n';
    }

    const std::string dosyaYolu = "kubys_faz1_veriler.bin";

    if (kubys::dosya::kubys_verilerini_dosyaya_kaydet(dosyaYolu, kitaplar, uyeler, oduncler)) {
        std::cout << "\nBinary dosyaya kaydedildi: " << dosyaYolu << '\n';
    }

    if (const auto okunan = kubys::dosya::kubys_verilerini_dosyadan_oku(dosyaYolu)) {
        std::cout << "Binary dosyadan okunan kitap sayisi: " << okunan->kitaplar.boyut() << '\n';
    }

    return 0;
}
