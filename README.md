---
layout: post
title: İmge Renklendirme
slug: image-colorization
author: Bahri ABACI
categories:
- Görüntü İşleme Uygulamaları
- Lineer Cebir
- Nümerik Yöntemler
references: "Colorization Using Optimization"
thumbnail: /assets/post_resources/image_colorization/thumbnail.png
---

Kısa bir süre önceki [İmge Renksizleştirme yazımızda]({% post_url 2015-10-16-imge-renksizlestirme %}) renkli imgelerin gri seviye dönüşümü için yenilikçi bir yöntem incelemiştik. Yöntem üç farklı kanaldan (R,G,B) oluşan renkli sayısal bir imgeyi tek bir kanala indirirken doku bilgisinin korunmasını da güvence altına almaktaydı. Bu yazımızda ise önceki yöntemin tersi olarak İmge Renklendirmeyi ele alacağız. İmge renklendirme (Image colorization) gri seviye kodlanmış bir imgeden renkli imge elde etmeye yarayan bir yöntemdir. Genellikle görselliği iyileştirmek için kullanılsa da gri seviye kameradan / sensörden alınan görüntülerin renklendirilmesi, eski fotoğrafların canlandırılması veya imge renklerinin değiştirilmesi gibi
uygulamalarda da kullanılmaktadır.  
  
<!--more-->

Çoğu görüntü işleme uygulamasında renkli imge 24 bitten oluşmakta ve yaklaşık 16 milyon renk barındırmaktadır. Gri seviye dönüşümü ile elde edilen tek kanallı imgede ise 8 bit ile tutulan 256 farklı gri tonu bulunmaktadır. Renk düzeyleri üzerinden bir hesaplama yapılırsa dönüşüm sonucunda renkli imgedeki yaklaşık 65 bin (16 milyon / 256 ) rengin tek bir siyahın tonu ile ifade edildiği görülür. Aşağıda verilen imgede yer alan bütün renklerin gri seviye dönüşümü sonucu aldığı değerler yaklaşık
aynıdır.  
  
![Eş Parlaklık Seviyeleri][isoluminance]
  
Bu çok yüksek veri sıkıştırmasına imge renklendirme açısından bakacak olursak, ortalamada tek bir siyahın tonuna karşılık olası 65 bin farklı renk bulunduğu görülür. Tek bir gözek için 65 bin farklı olasılığın bulunduğu bu problemde, akıllı bir yöntem kullanmadan 5x5 boyutunda bir imge için dahi hesaplamaları yapmak oldukça güçtür.  
  
Yazımızın konusu olan imge renklendirme için, literatürdeki en ilgi çekici yayın 2004 yılı Siggraph konferansında *Colorization using Optimization* yayını ile önerilmiştir. Çalışmada renkli imgedeki renkleri bulabilmek için imgenin bazı noktaları elle çizikler atarak renklendirilmiştir. Ardından yapılan bu renklendirmeler iteratif olarak gri seviyeler üzerine yayılarak imgenin tamamının renklenmesi sağlanmıştır. Aşağıda bir imgenin renklendirilmesine ilişkin adımlar gösterilmiştir.  
  
![Görüntü Renklendirme Adımları][steps]
  
Resimde ilk sırada renklendirmek istenen gri seviye imge yer almaktadır. İkinci sırada ise algoritmanın başlayabilmesi için elle işaretlenen kısmi renkli imge verilmiştir. Üçüncü sıradan itibaren de sırayla algoritmanın 50., 250. ve 1000. iterasyonlarına ait sonuçlar gösterilmiştir. Dikkatli bakılacak olursa elle işaretlenen renkler adım adım gri seviyeler üzerine yayılmakta ve bu sırada imgedeki doku ve parlaklık bilgisi de korunmaktadır.
  
Yöntemin çalışması iki temel prensibe dayanmaktadır.  
  
**1)** *Renklendirme işlemi ile elde edilen renkli imgenin ( yukarıda son sıradaki resim ) gri seviyesi, verilen gri imge ile birebir aynı olmak zorundadır.*  
  
Bu koşulu sağlamak için RGB uzayı yerine bir kanalı gri seviyeyi gösteren bir uzay (HSV, YUV, YIQ, vs.) kullanmak problemi oldukça basitleştirmektedir. Böyle bir uzay dönüşümü sonrasında aranan renkli imgenin gri seviye kanalı (HSV ise V, YUV ise Y kanalı) doğrudan giriş resmine eşitlenebilir. Renklerin aranması ise diğer iki kanal (HSV ise H ve S, YUV ise U ve V)  üzerinden yapılırsa, imgenin gri seviyesi değişmeden renkleri değiştirilebilir. Yazımızda bildiride de önerildiği gibi YUV renk uzayı kullanılmış ve dönüşüm için aşağıdaki kod satırları kullanılmıştır.

  
```c
counter = 0;
for(int n=0; n < N; n++) 
{
    for(int m=0; m < M; m++) 
    {

        Y[counter] = 0.00392156*(0.299*G.pixels[m][n].red+0.587*G.pixels[m][n].green+0.114*G.pixels[m][n].blue);
        U[counter] = 0.00392156*(0.596*I.pixels[m][n].red-0.274*I.pixels[m][n].green-0.322*I.pixels[m][n].blue);
        V[counter] = 0.00392156*(0.211*I.pixels[m][n].red-0.523*I.pixels[m][n].green+0.312*I.pixels[m][n].blue);
        counter++;
    }
}
```
  
  
**2)** *Belirli bir komşulukta yer alan iki gri seviye birbirine yakınsa, -renklendirme işlemi sonrası- renkleri de birbirine yakın olmalıdır.*  

Bu  prensip ise aşağıdaki formülasyon ile bir enerji optimizasyonu problemine dönüştürülebilir.

<p align="center"><img src="assets/post_resources/math//8c6f0daf99e32ba81dc79ab2ed3a6f04.svg?invert_in_darkmode" align=middle width=281.30945699999995pt height=62.53027769999999pt/></p>

Yukarıda verilen ifade de <img src="assets/post_resources/math//6bac6ec50c01592407695ef84f457232.svg?invert_in_darkmode" align=middle width=13.01596064999999pt height=22.465723500000017pt/>, YUV renk uzayının bir bileşenini göstermektedir ve çalışmada bu ifade <img src="assets/post_resources/math//a9a3a4a202d80326bda413b5562d5cd1.svg?invert_in_darkmode" align=middle width=13.242037049999992pt height=22.465723500000017pt/> için de çözülmektedir. İfade, bir <img src="assets/post_resources/math//89f2e0d2d24bcf44db73aab8fc03252c.svg?invert_in_darkmode" align=middle width=7.87295519999999pt height=14.15524440000002pt/> gözeğinin değerinin, kendisini çevreleyen <img src="assets/post_resources/math//0795a09b10f06d8180b24b652140f0cc.svg?invert_in_darkmode" align=middle width=35.65835789999999pt height=24.65753399999998pt/> komşuluğundaki gözeklerin ağırlıklı ortalamasına yakın olmasını zorlamaktadır. Burada <img src="assets/post_resources/math//2a00fc03470855726f65f9d16bf65480.svg?invert_in_darkmode" align=middle width=24.43030259999999pt height=14.15524440000002pt/>, komşu gözeğin merkez gözeğe olan uzaklığını ölçmek için kullanılan ağırlık metriğini göstermektedir ve 

<p align="center"><img src="assets/post_resources/math//ba5f2da3b79c2b615af6cdac8934cc06.svg?invert_in_darkmode" align=middle width=145.56621255pt height=19.51056195pt/></p>

formülü ile hesaplanmaktadır. Bu ifade basit şekilde <img src="assets/post_resources/math//89f2e0d2d24bcf44db73aab8fc03252c.svg?invert_in_darkmode" align=middle width=7.87295519999999pt height=14.15524440000002pt/> ve <img src="assets/post_resources/math//6f9bad7347b91ceebebd3ad7e6f6f2d1.svg?invert_in_darkmode" align=middle width=7.7054801999999905pt height=14.15524440000002pt/> noktaları arasındaki gri seviye farklılığı ölçmektedir.

Çalışmada komşuluk derecesi bir olarak seçilmiş ve herhangi bir gözeğin etrafını saran sekiz gözek komşu olarak değerlendirilmiştir. Aşağıda verilen kod satırlarında imgenin gözekleri tek tek dolanılmış ve renksiz olan her gözek için 8 komşusunun bu gözeğe olan gri seviye farklılıkları hesaplanmıştır. Hesaplanan bu değerler <img src="assets/post_resources/math//53d147e7f3fe6e47ee05b88b166bd3f6.svg?invert_in_darkmode" align=middle width=12.32879834999999pt height=22.465723500000017pt/> değişkeninde saklanmıştır.

  
```c
for(r=0; r < M*N; r++) 
{
    if( (U[r]*U[r] + V[r]*V[r]) < 0.00001 ) 
    {
        tsum = 0;
        for(k=0; k < 8; k++) {
        D[k] = exp ( -0.5*(Y[k2c(r,k,M,N)]-Y[k2c(r,k,M,N)])*(Y[k2c(r,k,M,N)]-Y[k2c(r,k,M,N)]) );
        tsum += D[k];
        }
        for(k=0; k < 8; k++) { A[k][r] = -D[k]/tsum; }
    }
}
```
  

Optimizasyon problemini çözmeye geçmeden önce matrisleri kullanarak enerji ifadesini daha sade ve çözümü daha kolay görülür şekilde yazalım. Yukarıda verilen enerji ifadesi 

<p align="center"><img src="assets/post_resources/math//82ec3f1250465663fffbabde1b00245d.svg?invert_in_darkmode" align=middle width=201.89981459999998pt height=18.312383099999998pt/></p>

matrissel biçiminde yazılabilir. Bu durumda enerji fonksiyonunu en küçükleyen <img src="assets/post_resources/math//f270c22e660f6744ca8d15f5c20bc78b.svg?invert_in_darkmode" align=middle width=33.50685194999999pt height=24.65753399999998pt/> vektörü de, bilinen <img src="assets/post_resources/math//315ed515a7438fe959a31cef786171d2.svg?invert_in_darkmode" align=middle width=33.67432859999999pt height=24.65753399999998pt/> ve <img src="assets/post_resources/math//c41cfef05aafb8ab29b6c3948a2ba4e9.svg?invert_in_darkmode" align=middle width=28.18692854999999pt height=22.465723500000017pt/> matrisleri kullanılarak

<p align="center"><img src="assets/post_resources/math//db9d169d34e1dd8ede081a55aa1bdfec.svg?invert_in_darkmode" align=middle width=124.55552669999999pt height=18.312383099999998pt/></p>

işlemi ile bulunur. Burada <img src="assets/post_resources/math//c41cfef05aafb8ab29b6c3948a2ba4e9.svg?invert_in_darkmode" align=middle width=28.18692854999999pt height=22.465723500000017pt/> matrisi her satırın da 8 komşu için ağırlıkları saklayan <img src="assets/post_resources/math//6498f82e0330e6a16f1d1e334e50bc20.svg?invert_in_darkmode" align=middle width=85.57058234999998pt height=22.465723500000017pt/> boyutlu bir seyrek matrisdir. <img src="assets/post_resources/math//c41cfef05aafb8ab29b6c3948a2ba4e9.svg?invert_in_darkmode" align=middle width=28.18692854999999pt height=22.465723500000017pt/> matrisini <img src="assets/post_resources/math//9f2b6b0a7f3d99fd3f396a1515926eb3.svg?invert_in_darkmode" align=middle width=36.52961069999999pt height=21.18721440000001pt/> boyutunda örnek bir imge üzerinden kavrayalım. Aşağıdaki resimde imgede yer alan her bir gözeğin (yeşil) komşuları (kırmızı) gösterilmiştir.  
  
![Görüntü Renklendirme Algoritması][neighbours]
  
Bu imgede yer alan gözekler için oluşturulan ağırlık matrisi <img src="assets/post_resources/math//c41cfef05aafb8ab29b6c3948a2ba4e9.svg?invert_in_darkmode" align=middle width=28.18692854999999pt height=22.465723500000017pt/> aşağıdaki şekilde olacaktır. Matriste birinci satır 1. gözeğin, ikinci satır 2. gözeğin, dokuzuncu satır 9. gözeğin komşularına olan uzaklıklarını barındıracaktır.  
  
<p align="center"><img src="assets/post_resources/math//ae032d32c3fc88d75a2f1051453a1ae0.svg?invert_in_darkmode" align=middle width=488.48682299999996pt height=180.8876454pt/></p>
  
Yukarıdaki formulasyonda da gösterildiği üzere yapmamız gereken işlemde bu matrisin tersine ihtiyacımız vardır. Matrisin boyutu (<img src="assets/post_resources/math//6498f82e0330e6a16f1d1e334e50bc20.svg?invert_in_darkmode" align=middle width=85.57058234999998pt height=22.465723500000017pt/>) resmin boyutunun (<img src="assets/post_resources/math//38c940e42b166347e72f8cc587bd9732.svg?invert_in_darkmode" align=middle width=32.73970589999999pt height=22.465723500000017pt/>) karesi ile hesaplandığından işlem yükü oldukça ağır sanılabilir ancak matrisin her satırında en fazla 8 eleman 0 dan farklı olduğundan uygun algoritmalarla tersi bulma işlemi oldukça hızlandırılabilir. Bu çalışmada Gauss-Seidel yöntemi kullanılarak <img src="assets/post_resources/math//6bac6ec50c01592407695ef84f457232.svg?invert_in_darkmode" align=middle width=13.01596064999999pt height=22.465723500000017pt/> ve <img src="assets/post_resources/math//a9a3a4a202d80326bda413b5562d5cd1.svg?invert_in_darkmode" align=middle width=13.242037049999992pt height=22.465723500000017pt/> kanalları bulunmuştur.  
  
### Gauss-Seidel

Gauss-Seidel yöntemi <img src="assets/post_resources/math//6ffa573707fca115cad7b243d91a7109.svg?invert_in_darkmode" align=middle width=50.69621369999999pt height=22.831056599999986pt/> şeklinde verilen bir doğrusal denklem takımını iteratif olarak çözmekte kullanılan bir yöntemdir. <img src="assets/post_resources/math//53d147e7f3fe6e47ee05b88b166bd3f6.svg?invert_in_darkmode" align=middle width=12.32879834999999pt height=22.465723500000017pt/>, <img src="assets/post_resources/math//0ef69de18444d6cd8f1e8e13faf27443.svg?invert_in_darkmode" align=middle width=50.091150449999994pt height=22.465723500000017pt/> matris, <img src="assets/post_resources/math//332cc365a4987aacce0ead01b8bdcc0b.svg?invert_in_darkmode" align=middle width=9.39498779999999pt height=14.15524440000002pt/> ve <img src="assets/post_resources/math//4bdc8d9bcfb35e1c9bfb51fc69687dfc.svg?invert_in_darkmode" align=middle width=7.054796099999991pt height=22.831056599999986pt/> <img src="assets/post_resources/math//f9c75bf2a73e8034bff58ed0c8e3c3c9.svg?invert_in_darkmode" align=middle width=43.31036984999999pt height=22.465723500000017pt/> vektörler olmak üzere aranan <img src="assets/post_resources/math//332cc365a4987aacce0ead01b8bdcc0b.svg?invert_in_darkmode" align=middle width=9.39498779999999pt height=14.15524440000002pt/> vektörü şu iteratif adımlarla bulunur.

<p align="center"><img src="assets/post_resources/math//6ad732a3ae468d397ede83d414aa8f63.svg?invert_in_darkmode" align=middle width=321.87625634999995pt height=59.1786591pt/></p>

Denklem de verilen <img src="assets/post_resources/math//53d147e7f3fe6e47ee05b88b166bd3f6.svg?invert_in_darkmode" align=middle width=12.32879834999999pt height=22.465723500000017pt/> matrisi imge renklendirme probleminde ağırlık matrisini gösterdiğinden ve çok az sayıda elemanı 0 dan farklı olduğundan, uygun kod yazılması durumunda çözüm için gerekli işlem sayısı oldukça az olacaktır. Aşağıda verilen kod satırlarında <img src="assets/post_resources/math//53d147e7f3fe6e47ee05b88b166bd3f6.svg?invert_in_darkmode" align=middle width=12.32879834999999pt height=22.465723500000017pt/> ve <img src="assets/post_resources/math//4bdc8d9bcfb35e1c9bfb51fc69687dfc.svg?invert_in_darkmode" align=middle width=7.054796099999991pt height=22.831056599999986pt/> matrisleri kullanılarak <img src="assets/post_resources/math//332cc365a4987aacce0ead01b8bdcc0b.svg?invert_in_darkmode" align=middle width=9.39498779999999pt height=14.15524440000002pt/> vektörü iteratif şekilde bulunur.

```c
void iterative_solver(double *A[9], double *b, int M, int N, int S) 
{
    double sum;
    int k,t,s, Max_Iter = 500;

    double *x  = (double*) calloc(S,sizeof(double));
    memcpy(x, b, S*sizeof(double));

    for(t=0; t < Max_Iter; t++) 
    {
        for(s=0; s < M*N; s++) 
        {
            sum = 0;
            for(k=0; k < 8; k++) {
                sum += A[k][s]*x[k2c(s,k,M,N)];
            }
            x[s] = (b[s]-sum);
        }
    }
    memcpy(b, x, S*sizeof(double));
    free(x);
}
```
  
Yukarıda verilen kod <img src="assets/post_resources/math//6bac6ec50c01592407695ef84f457232.svg?invert_in_darkmode" align=middle width=13.01596064999999pt height=22.465723500000017pt/> ve <img src="assets/post_resources/math//a9a3a4a202d80326bda413b5562d5cd1.svg?invert_in_darkmode" align=middle width=13.242037049999992pt height=22.465723500000017pt/> kanalları için çalıştırılıp sonuçlar bulunduktan sonra, renkli resim YUV uzayından RGB uzayına çevrilerek elde edilir.  
  
Çalışmanın sonuçlarına ait örnekler ve kaynaklar aşağıda paylaşılmıştır. Bu paylaşımda anlatımı ve kodlamayı kolaylaştırmak için orjinal yayından kısmen farklı formüller ve algoritmalar kullanılmıştır. Bu nedenle aşağıdaki örnek renklendirmelerde orjinal algoritmadan biraz farklı ama kabul edilebilir sonuçlar elde edilmiştir. Görüntülerde ilk resimler gri kaynağı, ikinci resimler renklendirme işlemi için yapılan işaretlemeleri, üçüncü imge ise yazımızda anlatılan algoritmanın sonucunu göstermektedir. Son sıradaki imge ise elde edilmeye çalışılan renkli imgedir.  
  
![Görüntü Renklendirme Örnek][example1]
![Görüntü Renklendirme Örnek][example2]
  
**Referanslar**  
* Levin, Anat, Dani Lischinski, and Yair Weiss. ["Colorization using optimization."](http://kucg.korea.ac.kr/new/course/2005/CSCE352/paper/levin04.pdf) ACM transactions on graphics (tog). Vol. 23. No. 3. ACM, 2004.
*  Gauss-Seidel method wikipedia article. ["https://www.wikiwand.com/en/Gauss–Seidel_method"](https://www.wikiwand.com/en/Gauss–Seidel_method)

[RESOURCES]: # (List of the resources used by the blog post)
[isoluminance]: /assets/post_resources/image_colorization/iso_lum.png
[steps]: /assets/post_resources/image_colorization/imge_renklendirme_ornek.png
[neighbours]: /assets/post_resources/image_colorization/tum_komsular.png
[example1]: /assets/post_resources/image_colorization/gray2color_example1.png
[example2]: /assets/post_resources/image_colorization/gray2color_example2.png