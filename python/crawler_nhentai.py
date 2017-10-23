import requests,os,shutil,re
from datetime import datetime
from bs4 import BeautifulSoup

headers = {"User-Agent":"Mozilla/5.0 (X11; Linux x86_64; rv:52.0) Gecko/20100101 Firefox/52.0",
        "Accept":"text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8"
        }

def now():
    return datetime.now().strftime("%H:%M:%S")

def download(urls):
    print(("[{}] [INFO] Total {} book/books").format(now(),len(urls)))
    for index,url in enumerate(urls,1):
        r=requests.get(url=url,headers=headers)                              #Get url
        r = BeautifulSoup(r.text,"html.parser")              #use bs4 and lxml 解析網頁
        dir_create(r)

    print("[%s] [INFO] All download completed!"%now())
    return


def dir_create(r):  # 創建新目錄
    bookname=r.find_all("h2")[0].text.replace("/","")
    print(("[{}] [INFO] Start downloading {} !").format(now(),bookname))
    dirname=os.getcwd()+'/'+bookname  #執行程式的地方創建目錄
    if(os.path.exists(dirname)):  #檢查目錄是否已存在
        print(("[{}] [INFO] {} existed!").format(now(),bookname))  
    else:  
        os.mkdir(dirname)         #不存在則按照要求再創建目錄
        print(("[{}] [INFO] {} created successfully!").format(now(),bookname))  
    download_picture(r,dirname)
    print(("[{}] [INFO] {} download completed!").format(now(),bookname))  
    return


def download_picture(r,dirname):
    amount=len(r.select('.gallerythumb'))
    print(("[{}] [INFO] Total {} pictures, Start downloading").format(now(),amount))
    for index,picture in enumerate(r.select('.gallerythumb .lazyload'),1):
        picture=picture['data-src']
        url = "https://i" + picture[9:-5]+picture[-4:]      #get the image url
        response = requests.get(url,stream=True,headers=headers)
        pt = '{dirname}/{}.jpg'.format(index,dirname=dirname)   #filename
        print(("[{}] [INFO] Start downloading: {} ").format(now(),url))
        with open(pt,'wb') as f:                            #save jpg
            shutil.copyfileobj(response.raw,f)
        print(("[{}] [INFO] {} download successfully！").format(now(),url))
    print(("[{}] [INFO] Total {} pictures download completed! ").format(now(),amount))
    return


if __name__ == "__main__":
    print("Please Enter url\nFor example:https://nhentai.net/g/xxxxxx\nmultiple download Please use space to Separate\nex:https://nhentai.net/g/xxxxxx https://nhentai.net/g/xxxxxx")
    urls = str(input())
    urls = re.split(" +",urls)
    for i,url in enumerate(urls):
        if url == '':
            urls.pop(i)
    download(urls)
