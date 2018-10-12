const { MongoClient } = require(`mongodb`)
const express = require(`express`)
const app = express()
const PORT = 2224
let DB

app.get('/data', (req, res) => {
    if (req.query.id && req.query.raw) {
        const data = {
            sensorID: parseInt(req.query.id),
            raw: parseInt(req.query.value),
            value: calCO(req.query.value),
            insertedTime: Date.now()
        }
        DB.collection(`data`).insert(data).then(() => {
            console.log(`Inserted ${req.query.id}`)
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