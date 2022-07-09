//LiteLoaderScript Dev Helper
/// <reference path="e:\MCServer\LXLDevHelper-VsCode\Library/Library/JS/Api.js" /> 

var port = 8089
var wsc = new WSClient();
var saveTime = 0;
logger.setTitle("CSPBot-Websocket")

function onText(msg){
    msg = msg.replaceAll("\n","")
    logger.debug(msg);
    var js = JSON.parse(msg)
    if(js["packet"] == "cmd"){
        if(js["data"] != ""){
            mc.runcmd(js["data"])
        }
    }
    else if(js["packet"] == "heart"){
        var heartTime = Number(js["data"])
        if(heartTime-saveTime > 8){
            logger.warn("CSPBot-Websocket 延迟过高")
        }
    }
}

function onError(data){
    logger.error("Websocket 出现错误，错误:"+data)
}

function onLost(code){
    logger.error("Websocket 链接丢失，请检查...")
}

function sendHeart(){
    var time = new Date().getTime().toString();
    saveTime = time;
    wsc.send(JSON.stringify({"packet":"heart","data":time}))
}

function connectSuccess(success){
    if(success){
        log("CSPBot-Websocket 连接成功.")
        wsc.listen("onTextReceived",onText)
        wsc.listen("onError",onError)
        wsc.listen("onLostConnection",onLost)
        setInterval(sendHeart,3*1000);
    }else{
        log("CSPBot-Websocket 连接失败")
    }
}

function connectServer(){
    wsc.connectAsync("ws://127.0.0.1:"+port,connectSuccess);
    return true;
}

mc.listen("onServerStarted",connectServer)