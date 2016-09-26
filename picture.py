import requests
from bs4 import BeautifulSoup
s=requests.session()
def request(web):
    r=s.get(url=web)
    r = BeautifulSoup(r.text,"lxml")
    grab_picture(r)
def grab_picture(self):
    a=1
    for picture in self.select('img[class="BDE_Image"]'):
        r=requests.get(picture['src'])
        filename = '/home/mino/Pictures/test/{0}.jpg'.format(a)
        with open(filename,'wb') as f:
            f.write(r.content)
        a=a+1
if __name__ == "__main__":
    url ="http://tieba.baidu.com/p/2166231880"
    request(url)
