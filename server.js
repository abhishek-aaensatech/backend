const express = require("express");
const cors = require("cors")
require('dotenv').config();
const DbConnect = require('./database.js');
const DetailsModel = require("./models/details.js");
const GatewayModel = require("./models/optimizer.js");
const ToggleModel = require("./models/toggleModel.js");
const bodyParser = require('body-parser');
let converter = require('json-2-csv');
const fs = require('fs');
const axios = require("axios");
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
    const queries = req.query;
    if (Object.keys(queries).length === 0) {
        return res.status(400).send("<p>Please Provide queries like <br><br> http://44.202.86.124:5000/logs?Date=2023-08-09</p>");
    }
    try {
        const allDetails = await DetailsModel.find(queries).sort({ createdAt: -1 });
        if (!allDetails || allDetails.length == 0)
            return res.status(404).send("No data is found for given Query");
        return res.status(200).json(allDetails);
    } catch (error) {
        return res.status(500).json({ message: "Internal Server Error" });
    }
})

app.get('/csv', async (req, res) => {
    const queries = req.query;
    if (Object.keys(queries).length === 0) {
        return res.status(400).send("<p>Please Provide queries like <br><br> http://44.202.86.124:5000/csv?Date=2023-08-09</p>");
    }
    try {
        const allDetails = await DetailsModel.find(queries);
        if (!allDetails || allDetails.length === 0) {
            return res.status(404).send("We Don't Have Data");
        }
        const date = new Date();
        const filename = `${date.toUTCString().slice(5, 11)}.csv`;
        const data = [];
        for (let val of allDetails) {
            data.push({
                Date: val['Date'],
                Time: val['Time'],
                OptimizerID: val["OptimizerID"],
                GatewayID: val['GatewayID'],
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
        // console.log("data",data);
        const csv = await converter.json2csv(data);
        const myPath = path.join(__dirname, filename);
        try {
            fs.writeFileSync(myPath, csv, () => {
                return res.status(200).json({ message: "Internal Server Error" });
            });
            return res.status(200).download(myPath, () => {
                setTimeout(() => {
                    fs.unlinkSync(myPath)
                }, 5000)
            })
        } catch (er) {
            throw er;
        }
    } catch (error) {
        return res.status(500).json({ message: "Internal Server Error" });
    }
});

app.post("/getLatestData", async (req, res) => {
    const { GatewayId, OptimizerId } = req.body;
    if ((!GatewayId) || (!OptimizerId))
        return res.status(400).json({ message: "Either GatewayID or OptimizerId not available" });
    try {
        const val = await DetailsModel.find({ GatewayID: GatewayId, OptimizerID: OptimizerId }).sort({ createdAt: -1 }).limit(1);
        if (val.length === 0)
            return res.status(404).json({ message: "No Data Found for given GatewayID and OptimizerID" });
        return res.status(200).json(val[0]);
    } catch (error) {
        return res.status(500).json({ message: "Internal Server Error" });
    }
})

app.post("/getGraphData", async (req, res) => {
    const { GatewayId, OptimizerId } = req.body;
    console.log("OK1");
    if ((!GatewayId) || (!OptimizerId))
        return res.status(400).json({ message: "Either GatewayID or OptimizerId not available" });
    try {
        console.log("OK2");
        const val = await DetailsModel.find({ GatewayID: GatewayId, OptimizerID: OptimizerId }).sort({ createdAt: 1 }).limit(100);
        if (val.length === 0)
            return res.status(404).json({ message: "No Data Found for given GatewayID and OptimizerID" });
        console.log("OK3");
        return res.status(200).json(val);
    } catch (error) {
        return res.status(500).json({ message: "Internal Server Error" })
    }
    console.log("OK4");
})

app.post("/gatewayIDall", async (req, res) => {
    const { GatewayId } = req.body;
    if (!GatewayId)
        return res.status(400).json({ message: "Either GatewayID not available" });
    try {
        const val = await DetailsModel.find({ GatewayID: GatewayId });
        if (val.length === 0)
            return res.status(404).json({ message: "No Data Found for given GatewayID" });
        return res.status(200).json(val);
    } catch (error) {
        return res.status(500).json({ message: "Internal Server Error" })
    }
})


async function saveData(detailsOfOne) {
    try {
        const detail = detailsOfOne;
        if (!detail.GatewayID) {
            console.log("not");
            return;
        }
        const gateway = await GatewayModel.find({ GatewayID: detail.GatewayID });
        if (gateway.length === 0) {
            if (detail.OptimizerID) {
                const details = {
                    GatewayID: detail.GatewayID,
                    OptimizerIds: [detail.OptimizerID]
                };
                await GatewayModel.create(details);
            } else {
                const details = {
                    GatewayID: detail.GatewayID,
                    OptimizerIds: []
                };
                await GatewayModel.create(details);
            }
        } else {
            if (detail.OptimizerID && (!(gateway[0].OptimizerIds.includes(detail.OptimizerID)))) {
                const newOptimizerIds = [...gateway[0].OptimizerIds, detail.OptimizerID]
                await GatewayModel.updateOne({ GatewayID: detail.GatewayID }, { $set: { OptimizerIds: newOptimizerIds } });
            }
        }
        const res = await DetailsModel.create(detail);
        console.log("saved");
    } catch (error) {
        console.log(error);
        throw error;
    }

}


app.post('/sendNewDetails', async (req, res) => {
    const detail = req.body;
    if (!detail.GatewayId || !detail.OptimizerId)
        return res.status(404).json({ message: "Data is missing" });
    try {
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
    } catch (error) {
        return res.status(500).json({ message: "Internal Server Error" })
    }

})

app.post('/sendNewDetailsFromGateway', async (req, res) => {
    try {
        console.log(req.body);
        const { data, meterDetails } = req.body;
        if (data.length === 0) {
            saveData({ ...meterDetails });
        } else {
            for (const detailsOfOne of data) {
                saveData({ ...detailsOfOne, ...meterDetails });
            }
        }
    } catch (error) {
        return res.status(500).json({ message: "Internal Server Error" })
    }

    return res.status(200).json({ message: "We Receive Data" });
})

app.post("/addOptimizer", async (req, res) => {
    const { GatewayId, OptimizerId } = req.body;
    if ((!GatewayId) || (!OptimizerId))
        return res.status(400).json({ message: "Please provide a GatewayID and OptimizerID" });
    const gateway = await GatewayModel.find({ GatewayID: GatewayId });
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
    try {
        const gateway = await GatewayModel.find({ GatewayID: GatewayId });
        if (gateway.length === 0)
            return res.status(404).json({ message: "No Data Found for given GatewayID" });
        return res.status(200).json(gateway);
    } catch (error) {
        return res.status(500).json({ message: "Internal Server Error" });
    }
})

app.post("/toggleOptimizer", async (req, res) => {
    try {
        const { GatewayID } = req.body;
        const detail = await gatewayIPmap.find({ GatewayID });
        const { IP } = detail[0];
        if (!IP)
            return res.json({ message: "GatewayID does not exist" });
        const response = await axios.post(`http://${IP}:8080/toggleOptimizer`, req.body)
        console.log(response);
        return res.status(200).json({ message: "We received you request" });
    } catch (error) {
        return res.status(500).json({ message: "Internal Server Error" });
    }
})

app.get("/allGateways", async (req, res) => {
    try {
        const val = await GatewayModel.find({});
        if (val.length === 0)
            return res.status(404).json({ message: "No data Found" });
        return res.status(200).json(val);
    } catch (error) {
        return res.status(500).json({ message: "Internal Server Error" });
    }
})

app.post("/toggleOptimizerIDDashboard", async (req, res) => {
    try {
        const { GatewayID, OptimizerID, Flag } = req.body;
        if (!GatewayID || !OptimizerID || !Flag)
            return res.status(500).json({ message: "Please provide GatewayID, OptimizerID, and Flag" });
        await ToggleModel.findOneAndUpdate({ GatewayID, OptimizerID }, { Flag }, { upsert: true });
        res.status(200).json({ message: "We Received your request and start processing" });
    } catch (error) {
        console.log(error);
        res.status(500).json({ message: "Internal Server Error" });
    }
})

app.post("/toggleOptimizerIDGateway", async (req, res) => {
    try {
        console.log(req.body);
        if (!req.body || !req.body.GatewayID)
            return res.status(500).json({ message: "Please provide GatewayID" });
        const data = await ToggleModel.find(req.body);
        const arr = [];
        for(const d of data){
            const {OptimizerID,Flag} = d;
            arr.push({OptimizerID,Flag})
        }
        await ToggleModel.deleteMany(req.body);
        res.status(200).json(arr);
    } catch (error) {
        console.log(error);
        res.status(500).json({ message: "Internal Server Error" });
    }
})

app.listen(5000, () => {
    console.log('server is running on port 5000');
}); 