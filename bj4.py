import requests,os
from bs4 import BeautifulSoup
def request(web):
    r=requests.get(url=web)                                        #Get url
    r = BeautifulSoup(r.text,"lxml")                        #use bs4 and lxml 解析網頁
    dir_create(r)

def dir_create(r):  # 創建新目錄
    bookname=r.find_all("h2")[0].text.replace("/","")
    dirname= '/home/mino/Pictures/GTFO/'+bookname #自己的目錄
    #dirname=os.getcwd()+'/'+bookname  #其他人請註解掉上面使用這個 或者 自行修改上面目錄
    if(os.path.exists(dirname)):  #檢查目錄是否已存在
        print("目錄已存在！")  
    else:  
        os.mkdir(dirname)         #不存在則按照要求再創建目錄
        print("目錄創建成功！")
    save_picture(r,dirname)


def save_picture(r,dirname):
    a=1                                                     # set filename number
    amount=len(r.select('img[class]'))
    print(("圖片總共數量{0}張,開始下載").format(amount))
    for picture in r.select('img[class]'):
        picture=picture['data-src']
        picture="http:"+picture[:-5]+picture[-4:]
        r=requests.get(picture)                      #get the image url
        pt = '{dirname}/{0}.jpg'.format(a,dirname=dirname)   #filename
        with open(pt,'wb') as f:                            #save jpg
            f.write(r.content)
        print(("抓取完第{0}張圖片").format(a))
        a=a+1
    print(("已抓取完畢{0}張圖片").format(a-1))
if __name__ == "__main__":
    url = input("安安 請輸入網址:")
    request(url)

