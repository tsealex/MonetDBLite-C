(: q4 'SELECT alb.Title, ass.Title FROM Assets as ass, Albums as alb WHERE ass.AlbumId=alb.AlbumId' :)
import module namespace music = "http://www.cwi.nl/~boncz/music/opt/" at "http://www.cwi.nl/~boncz/music/opt/music.mil";
music:AlbumAsset("music.xml", 1000000000) 
