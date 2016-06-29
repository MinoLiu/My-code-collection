#This is use BeautifulSoup to collect our school's dormnet data
#Help me to get the information
import getpass
import requests
from bs4 import BeautifulSoup #use python-requests and python-bs4
username = input("username")
password = getpass.getpass("password: ")
data ={
        '_method=POST':'',
        'data[User][username]':username,
        'data[User][password]':password,
        }
s=requests.session()
s.post(url='http://register.dormnet.yuntech.edu.tw/login',data=data)
r=s.get(url='http://register.dormnet.yuntech.edu.tw/NetFlow/apiFlowCheck.json')
c=s.get(url='http://register.dormnet.yuntech.edu.tw/user-status')
soup=BeautifulSoup(r.text,"lxml")
check=BeautifulSoup(c.text,"lxml")
status= check.select("dd")[3].text
apartment=check.select("dd")[1].text
name=check.select("strong")[0].text
print("Welcome ",name)
print("您住的床位為",apartment)
print("你的狀態 :",status)
netflow= soup.select("p")[0].text
netflow=eval(netflow)
netflow=netflow['netFlowInfo']
netflow=netflow[0]
netflow=netflow['NetFlow']
date=netflow['Date']
income=netflow['InCome']
OutCome=netflow['OutCome']
print('今日は',date)
total=int(income)+int(OutCome)
total /= 1024 * 1024 *1024
print('流量統計',round(total,3),'GB')
print('あと',round(8.35-total,3),'GBです')
