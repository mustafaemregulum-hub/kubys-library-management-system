# KUBYS Proje Sunum Notları

**Hazırlayan:** Mustafa Emre Gülüm

## 1. Giriş

Bu proje, Yazılım Geliştirme II dersi kapsamında geliştirilen KUBYS Kütüphane Yönetim Sistemi uygulamasıdır. Amaç; kitap kataloğu, üye yönetimi, ödünç alma ve iade süreçlerini C++20 ve Qt6 kullanarak yönetebilen bir sistem oluşturmaktır.

## 2. Faz 1 - Veri Katmanı

İlk fazda Qt kullanılmadan C++20 ve STL tabanlı veri katmanı hazırlanmıştır. Kitap, üye ve ödünç kaydı için ayrı struct yapıları tanımlanmıştır. Ödünç kaydında iade tarihi henüz belli olmayabileceği için `std::optional` kullanılmıştır.

Veriler `Depo<Anahtar, Deger>` isimli şablon sınıfta `std::map` ile tutulur. Bu sınıf ekleme, silme, arama, güncelleme, tüm kayıtları alma ve lambda ile filtreleme işlemlerini destekler.

Ayrıca STL/ranges ile kategoriye göre kitap listeleme, gecikmiş kayıtları bulma, kitapları yayın yılına göre sıralama, bir üyenin ödünç sayısını hesaplama ve en çok ödünç alınan kitabı bulma sorguları yazılmıştır.

## 3. Faz 1 - Binary Dosya İşlemleri

Verilerin kalıcı olması için binary dosya okuma/yazma fonksiyonları eklenmiştir. Uygulama açılırken kayıtlı binary dosya varsa okunur. Kapanırken mevcut veriler otomatik kaydedilir. Böylece program kapansa bile kitap, üye ve ödünç kayıtları korunur.

## 4. Faz 2 - Qt6 Arayüz

İkinci fazda Qt6 Widgets ile grafiksel arayüz geliştirilmiştir. Ana pencerede menü, araç çubuğu, durum çubuğu ve sekmeli görünüm vardır. Kitaplar, üyeler, ödünç kayıtları ve grafik ayrı sekmelerde gösterilir.

Tablolar için `QAbstractTableModel`, arama/filtreleme için `QSortFilterProxyModel` kullanılmıştır. Kategoriye göre kitap dağılımı `QChart` ile grafik olarak gösterilir.

## 5. Ödünç Alma ve İade Kontrolleri

Ödünç verme sırasında üyenin varlığı, kitabın varlığı, uygun kopya sayısı ve aynı üyenin aynı kitabı zaten ödünç alıp almadığı kontrol edilir. Başarılı ödünç işleminde kitabın kopya sayısı azaltılır. İade işleminde kayıt kapatılır, iade tarihi atanır ve kopya sayısı artırılır.

## 6. Faz 3 - Çok Kanallı Programlama

CSV katalog yükleme ve gecikme raporu gibi uzun işlemler arka planda çalıştırılır. Bunun için QThread, std::thread, mutex, lock_guard ve atomic_bool kullanılmıştır. Kullanıcı işlem sırasında ilerleme yüzdesini görebilir ve işlemi iptal edebilir.

## 7. Faz 4 - Ağ Programlama

Son fazda TCP tabanlı istemci/sunucu mimarisi eklenmiştir. Sunucu `QTcpServer`, istemci `QTcpSocket` kullanır. İstek ve yanıtlar JSON formatındadır. Sunucu birden fazla istemciyi kabul eder ve ödünç/iade işlemlerinde çakışma kontrolü yapar.

## 8. Sonuç

KUBYS projesi dönem boyunca veri katmanından grafiksel arayüze, çok kanallı programlamadan ağ mimarisine kadar aşamalı olarak geliştirilmiştir. Proje C++20, STL, Qt6, thread yönetimi ve TCP soket programlama konularını bir arada uygulamaktadır.
