# KUBYS - Kütüphane Yönetim Sistemi

## Gönderen / Hazırlayan

- Mustafa Emre Gülüm

## Proje Hakkında

Bu proje, Yazılım Geliştirme II dersi kapsamında geliştirilen KUBYS Kütüphane Yönetim Sistemi çalışmasıdır. Proje C++20 standardı ile hazırlanmıştır ve dört fazı kapsayacak şekilde düzenlenmiştir.

## Proje Durumu

- Faz 1: STL veri katmanı, `Depo` şablon sınıfı, `std::optional`, STL/ranges sorguları ve binary dosya işlemleri eklendi.
- Faz 2: Qt6 Widgets tabanlı grafiksel arayüz, formlar, tablo modelleri, arama/filtreleme, QSettings ve QChart eklendi.
- Faz 3: QThread, `std::thread`, `std::mutex`, `std::lock_guard`, ilerleme bildirimi ve iptal mekanizması eklendi.
- Faz 4: QTcpServer/QTcpSocket ile TCP sunucu-istemci yapısı ve JSON mesaj protokolü eklendi.

## Klasör Yapısı

```text
KUBYS/
├── CMakeLists.txt
├── README.md
├── docs/
│   └── SUNUM_NOTLARI.md
├── include/
│   ├── depo.h
│   ├── dosya_islemleri.h
│   ├── faz1_sorgular.h
│   └── varliklar.h
└── src/
    ├── faz1_demo.cpp
    ├── main.cpp
    ├── main_window.h / main_window.cpp
    ├── kitap_model.h / kitap_model.cpp
    ├── uye_model.h / uye_model.cpp
    ├── odunc_model.h / odunc_model.cpp
    ├── kitap_dialog.h / kitap_dialog.cpp
    ├── uye_dialog.h / uye_dialog.cpp
    ├── odunc_dialog.h / odunc_dialog.cpp
    ├── arka_plan_isci.h / arka_plan_isci.cpp
    ├── kutuphane_sunucu.h / kutuphane_sunucu.cpp
    └── ag_istemci.h / ag_istemci.cpp
```

## Faz 1 - STL ve Veri Katmanı

`include/varliklar.h` içinde temel varlıklar tanımlanmıştır:

- `Kitap`
- `Uye`
- `OduncKaydi`
- `OduncDurum`

`OduncKaydi` içindeki `iade_tarihi` alanı `std::optional<std::string>` olarak tutulur. Böylece henüz iade edilmemiş kayıtlar `std::nullopt` ile temsil edilir.

`include/depo.h` içinde genel amaçlı `Depo<Anahtar, Deger>` şablon sınıfı vardır. Temel işlemler:

- `ekle()`
- `bul()`
- `sil()`
- `guncelle()`
- `var_mi()`
- `tumunu_al()`
- `boyut()`
- `temizle()`
- `filtrele()`

`include/faz1_sorgular.h` içinde istenen STL/ranges sorguları ayrı fonksiyonlar halinde yazılmıştır:

- Belirli kategorideki kitapları listeleme
- Gecikmiş iade kayıtlarını bulma
- Kitapları yayın yılına göre sıralama
- Bir üyenin toplam ödünç aldığı kitap sayısını hesaplama
- En çok ödünç alınan kitabı bulma

`include/dosya_islemleri.h` içinde binary dosya işlemleri vardır:

- `kubys::dosya::dosyaya_kaydet(...)`
- `kubys::dosya::dosyadan_oku<...>(...)`
- `kubys::dosya::kubys_verilerini_dosyaya_kaydet(...)`
- `kubys::dosya::kubys_verilerini_dosyadan_oku(...)`

`src/faz1_demo.cpp`, Faz 1 için Qt kullanmadan çalışan test/demonstrasyon dosyasıdır.

## Faz 2 - Qt6 Grafiksel Arayüz

Qt6 Widgets ile `QMainWindow` tabanlı masaüstü arayüz oluşturulmuştur. Arayüzde şu bölümler vardır:

- Menü çubuğu
- Araç çubuğu
- Durum çubuğu
- Sekmeli görünüm
- Kitaplar, Üyeler, Ödünç Kayıtları ve Grafik sekmeleri

Her temel veri için `QAbstractTableModel` tabanlı model sınıfı vardır:

- `KitapModel`
- `UyeModel`
- `OduncModel`

Arama/filtreleme için `QSortFilterProxyModel` kullanılmıştır. Kategoriye göre kitap dağılımı `QChart` ve `QPieSeries` ile gösterilir.

## Ödünç Alma ve İade Mantığı

Yerel arayüzde ödünç verme sırasında şu kontroller yapılır:

- Üye sistemde var mı?
- Kitap sistemde var mı?
- Kitabın uygun kopyası var mı?
- Aynı üye aynı kitabı hâlâ ödünçte tutuyor mu?

Ödünç verme başarılı olursa kitabın `kopya_sayisi` 1 azaltılır. İade işleminde kayıt `IadeEdildi` yapılır, `iade_tarihi` güncellenir ve kitabın `kopya_sayisi` 1 artırılır.

## Kalıcılık

Uygulamada iki tür kalıcılık vardır:

1. **Binary veri dosyası:** Uygulama açılırken `kubys_veriler.bin` otomatik okunur. Uygulama kapanırken mevcut veriler aynı dosyaya otomatik kaydedilir.
2. **JSON dışa aktarma/içe aktarma:** Menüden JSON dosyası kaydedilebilir veya yüklenebilir.

Pencere konumu ve boyutu `QSettings` ile saklanır.

## Faz 3 - Çok Kanallı Programlama

Uzun süren işlemler arayüzü dondurmamak için arka planda yürütülür:

- CSV katalog yükleme
- Gecikme raporu oluşturma

Kullanılan yapılar:

- `QThread`
- `std::thread`
- `std::mutex`
- `std::lock_guard`
- `std::atomic_bool`
- İlerleme sinyali
- İptal mekanizması

## Faz 4 - Ağ Programlama

Ağ modülünde TCP tabanlı sunucu-istemci mimarisi vardır.

Sunucu:

- `QTcpServer` kullanır.
- Birden fazla istemci kabul eder.
- JSON tabanlı istek/yanıt protokolü kullanır.
- Ödünç alma ve iade işlemlerinde çakışma kontrolü yapar.

İstemci:

- `QTcpSocket` kullanır.
- Sunucuya bağlanır.
- Kimlik doğrulama, uzaktan kitap arama, uzaktan ödünç alma ve uzaktan iade işlemlerini gönderir.
- Bağlantı koparsa yeniden bağlanmayı dener.

Örnek JSON istek:

```json
{
  "tip": "odunc_al",
  "uye_no": 2001,
  "isbn": "978-975-1000-01-1"
}
```

## Derleme

### Sadece Faz 1 demosunu derlemek

Qt6 kurulu değilse bu yöntem kullanılabilir:

```bash
mkdir build
cd build
cmake .. -DKUBYS_BUILD_QT=OFF
cmake --build .
./kubys_faz1_demo
```

Alternatif doğrudan derleme:

```bash
g++ -std=c++20 -Wall -Wextra -Wpedantic -Iinclude -o kubys_faz1_demo src/faz1_demo.cpp
./kubys_faz1_demo
```

### Tam Qt uygulamasını derlemek

Qt6 kurulu sistemde:

```bash
mkdir build
cd build
cmake .. -DCMAKE_PREFIX_PATH=/path/to/Qt/6.x
cmake --build .
./kubys
```

Windows/MinGW örneği:

```powershell
mkdir build
cd build
cmake .. -G "MinGW Makefiles" -DCMAKE_PREFIX_PATH="C:\Qt\6.5.0\mingw_64"
cmake --build .
.\kubys.exe
```

## Kullanım Özeti

1. Uygulama açıldığında kayıtlı binary veri dosyası varsa otomatik yüklenir.
2. Kayıtlı dosya yoksa örnek veriler oluşturulur.
3. Menü veya araç çubuğundan kitap, üye ve ödünç kaydı eklenebilir.
4. `İade Et` işlemi ile ödünç kayıtları kapatılabilir.
5. JSON veya binary formatta veri kaydedilip yüklenebilir.
6. CSV katalog yükleme ve gecikme raporu arka planda çalışır.
7. Ağ menüsünden sunucu başlatılabilir ve istemci bağlantısı denenebilir.
