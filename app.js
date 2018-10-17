const { MongoClient } = require(`mongodb`)
const express = require(`express`)
const app = express()
const PORT = 2224
let DB

app.get('/data', (req, res) => {
    if (req.query.id && req.query.raw) {
        let insertedTime = req.query.time ? Number(req.query.time) : Date.now()
        const data = {
            sensorID: parseInt(req.query.id),
            raw: req.query.ppm ? parseInt(req.query.ppm) : parseInt(req.query.raw),
            value: req.query.ppm ? calCOPPM(req.query.ppm) : calCO(req.query.raw) ,
            insertedTime,
        }
        if(req.query.id == 6){
            data.t = parseFloat(req.query.t)
            data.h = parseFloat(req.query.h)
        }
        DB.collection(`data`).insert(data).then(() => {
            //console.log(`Inserted ${req.query.id}`)
            res.send({ error: false })
        }).catch(error => {
            console.log(`DB Error`, error)
            res.send({ error: true })
        })
    } else if (req.query.id) {
        DB.collection(`data`).findOne({ sensorID: parseInt(req.query.id) }).then(data => {
            res.send(data)
        }).catch(error => {
            console.log(`DB Error`, error)
            res.send({ error: true })
        })
    } else {
        res.send({ error: true })
    }
})

MongoClient.connect(`mongodb://127.0.0.1/co`).then(db => {
    DB = db
    app.listen(PORT)
    console.log(`Listened port ${PORT}`)
}).catch(console.log)

function calCO(co, t = 25) {
    const MAX = 10000
    const MIN = 10
    let ppb = parseInt(co) * ((MAX - MIN) / 1024) * 1000
    return (ppb * 12.187 * 28) / (273.15 + t)
}

function calCOPPM(ppm, t = 25){
    return (ppm * 1000 * 12.187 * 28) / (273.15 + t)
}