import requests,os,shutil,re
from bs4 import BeautifulSoup

headers = {"User-Agent":"Mozilla/5.0 (X11; Linux x86_64; rv:52.0) Gecko/20100101 Firefox/52.0",
        "Accept":"text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8"
        }

def download(urls):
    print(("總共{0}本").format(len(urls)))
    for index,url in enumerate(urls,1):
        print(("開始下載第{0}本!").format(index))
        r=requests.get(url=url,headers=headers)                              #Get url
        r = BeautifulSoup(r.text,"html.parser")              #use bs4 and lxml 解析網頁
        dir_create(r)
    print("全部下載完成")


def dir_create(r):  # 創建新目錄
    bookname=r.find_all("h2")[0].text.replace("/","")
    dirname=os.getcwd()+'/'+bookname  #執行程式的地方創建目錄
    if(os.path.exists(dirname)):  #檢查目錄是否已存在
        print("目錄已存在！")  
    else:  
        os.mkdir(dirname)         #不存在則按照要求再創建目錄
        print("目錄創建成功！")
    save_picture(r,dirname)


def save_picture(r,dirname):
    amount=len(r.select('.gallerythumb'))
    print(("圖片總共數量{0}張,開始下載").format(amount))
    for index,picture in enumerate(r.select('.gallerythumb .lazyload'),1):
        picture=picture['data-src']
        response = requests.get("https://i" + picture[9:-5]+picture[-4:],stream=True,headers=headers)        #get the image url
        pt = '{dirname}/{0}.jpg'.format(index,dirname=dirname)   #filename
        with open(pt,'wb') as f:                            #save jpg
            shutil.copyfileobj(response.raw,f)
        print(("抓取完第{0}張圖片").format(index))
    print(("已抓取完畢{0}張圖片").format(amount))


if __name__ == "__main__":
    print("請輸入網址\n格式:https://nhentai.net/g/xxxxxx\n批量下載請用空格分隔網址\nex:https://nhentai.net/g/xxxxxx https://nhentai.net/g/xxxxxx")
    urls = str(input("安安 請輸入網址:\n"))
    urls = re.split(" +",urls)
    for i,url in enumerate(urls):
        if url == '':
            urls.pop(i)
    download(urls)
