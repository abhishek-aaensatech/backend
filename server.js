const express = require("express");
const cors = require("cors")
require('dotenv').config();
const DbConnect = require('./database.js');
const DetailsModel = require("./models/details.js");
const GatewayModel = require("./models/optimizer.js");
const bodyParser = require('body-parser');
let converter = require('json-2-csv');
const fs = require('fs');
const path = require("path");

DbConnect();

const PORT = process.env.PORT || 5000;
const app = express();

app.use(cors())
app.use(bodyParser.urlencoded({ extended: false }))
app.use(bodyParser.json())


app.get('/', (req, res) => {
    return res.send("Server started");
})

app.get("/logs", async (req, res) => {
    const allDetails = await DetailsModel.find({});
    if (!allDetails || allDetails.length == 0)
        return res.status(404).json({});
    return res.status(200).json(allDetails);
})

app.get('/csv', async (req, res) => {
    const allDetails = await DetailsModel.find({});
    if (!allDetails || allDetails.length === 0) {
        return res.status(404).json({});
    }
    const date = new Date();
    const filename = `${date.toUTCString().slice(5, 11)}.csv`;
    const data = [];
    for (let val of allDetails) {
        data.push({
            Date: val['Date'],
            Time: val['Time'],
            OptimizerId: val["OptimizerId"],
            GatewayId: val['GatewayId'],
            OptimizerMode: val['OptimizerMode'],
            RoomTemperature: val['RoomTemperature'],
            CoilTemperature: val['CoilTemperature'],
            'Ph1Voltage(V)': val['Ph1Voltage(V)'],
            'Humidity(%)': val['Humidity(%)'],
            'Ph1Current(A)': val['Ph1Current(A)'],
            'Ph1ActivePower(kW)': val['Ph1ActivePower(kW)'],
            Ph1PowerFactor: val['Ph1PowerFactor'],
            'Ph2Voltage(V)': val['Ph2Voltage(V)'],
            'Ph2Current(A)': val['Ph2Current(A)'],
            'Ph2ActivePower(kW)': val['Ph2ActivePower(kW)'],
            Ph2PowerFactor: val['Ph2PowerFactor'],
            'Ph2ApparentPower(kVA)': val['Ph2ApparentPower(kVA)'],
            'Ph3Voltage(V)': val['Ph3Voltage(V)'],
            'Ph3Current(A)': val['Ph3Current(A)'],
            'Ph3ActivePower(kW)': val['Ph3ActivePower(kW)'],
            Ph3PowerFactor: val['Ph3PowerFactor'],
        });
    }
    const csv = await converter.json2csv(data);
    const myPath = path.join(__dirname, filename);
    try {
        fs.writeFileSync(myPath, csv);
        res.status(200).download(myPath, () => {
            setTimeout(() => {
                fs.unlinkSync(myPath)
            }, 5000)
        })
    } catch (er) {
        console.log(er);
    }
});

app.post("/getLatestData", async (req, res) => {
    const { GatewayId, OptimizerId } = req.body;
    if ((!GatewayId) || (!OptimizerId))
        return res.status(400).json({ message: "Either GatewayID or OptimizerId not available" });
    const val = await DetailsModel.find({ GatewayId, OptimizerId }).sort({ "created_at": -1 });
    if (val.length === 0)
        return res.status(404).json({ message: "No Data Found for given GatewayID and OptimizerID" });
    return res.status(200).json(val[0]);
})

app.post("/getGraphData", async (req, res) => {
    const { GatewayId, OptimizerId } = req.body;
    if ((!GatewayId) || (!OptimizerId))
        return res.status(400).json({ message: "Either GatewayID or OptimizerId not available" });
    const val = await DetailsModel.find({ GatewayId, OptimizerId }).sort({ "created_at": 1 });
    if (val.length === 0)
        return res.status(404).json({ message: "No Data Found for given GatewayID and OptimizerID" });
    return res.status(200).json(val);
})

app.post("/gatewayIDall", async (req, res) => {
    const { GatewayId } = req.body;
    if (!GatewayId)
        return res.status(400).json({ message: "Either GatewayID not available" });
    const val = await DetailsModel.find({ GatewayId });
    if (val.length === 0)
        return res.status(404).json({ message: "No Data Found for given GatewayID" });
    return res.status(200).json(val);
})

app.post('/sendNewDetails', async (req, res) => {
    const detail = req.body;
    if (!detail.GatewayId || !detail.OptimizerId)
        return res.status(404).json({ message: "Data is missing" });
    const gateway = await GatewayModel.find({ GatewayId: detail.GatewayId });
    if (gateway.length === 0) {
        const details = {
            GatewayId: detail.GatewayId,
            OptimizerIds: [detail.OptimizerId]
        };
        await GatewayModel.create(details);
    } else {
        if (!(gateway[0].OptimizerIds.includes(detail.OptimizerId))) {
            const newOptimizerIds = [...gateway[0].OptimizerIds, detail.OptimizerId]
            await GatewayModel.updateOne({ GatewayId: detail.GatewayId }, { $set: { OptimizerIds: newOptimizerIds } });
        }
    }

    const val = await DetailsModel.create(detail);
    if (!val)
        return res.status(504).json({ message: "Internal Server Error and data has not saved" });
    return res.status(200).json(val);
})

app.post("/addOptimizer", async (req, res) => {
    const { GatewayId, OptimizerId } = req.body;
    if ((!GatewayId) || (!OptimizerId))
        return res.status(400).json({ message: "Please provide a GatewayID and OptimezerID" });
    const gateway = await GatewayModel.find({ GatewayId });
    if (gateway.length === 0) {
        const detail = {
            GatewayId,
            OptimizerIds: [OptimizerId]
        };
        const val = await GatewayModel.create(detail);
        if (!val)
            return res.status(504).json({ message: "Internal Server Error and data has not saved" });
        return res.status(200).json(val);
    } else {
        const newOptimizerIds = [...gateway[0].OptimizerIds, OptimizerId]
        const val = await GatewayModel.updateOne({ GatewayId }, { $set: { OptimizerIds: newOptimizerIds } });
        if (!val)
            return res.status(504).json({ message: "Internal Server Error and data has not saved" });
        return res.status(200).json(val);
    }
})

app.post('/getOptimizer', async (req, res) => {
    const { GatewayId } = req.body;
    if (!GatewayId)
        return res.status(400).json({ message: "Please provide a valid GatewayID" });
    const gateway = await GatewayModel.find({ GatewayId });
    if (gateway.lenght === 0)
        return res.status(404).json({ message: "No Data Found for given GatewayID" });
    return res.status(200).json(gateway);
})

app.get("/allGateways", async (req, res) => {
    const val = await GatewayModel.find();
    if (val.length === 0)
        return res.status(404).json({ message: "No data Found" });
    return res.status(200).json(val);
})

app.post("/toggleOptimizer",(req,res)=>{
    const {toggle} = req.body;
    if(!toggle){
        return res.send("Data bhej bhai");
    }
    res.json(req.body);
})

app.listen(PORT, () => {
    console.log("Server started on PORT ", PORT);
})  