const express = require("express");
const cors = require("cors")
require('dotenv').config();
const DbConnect = require('./database.js');
const DetailsModel = require("./models/details.js");
const GatewayModel = require("./models/optimizer.js");
const bodyParser = require('body-parser');

DbConnect();

const PORT = process.env.PORT || 5000;
const app = express();

app.use(cors())  
app.use(bodyParser.urlencoded({ extended: false }))
app.use(bodyParser.json())

 
app.get('/',(req,res)=>{
    return res.send("Server started");
})

app.get("/logs",async (req,res)=>{
    const allDetails = await DetailsModel.find({});
    if(!allDetails)
        return res.status(404).json({});
    return res.status(200).json(allDetails);
})

app.get('/csv',async(req,res)=>{
    const allDetails = await DetailsModel.find({});
    if(!allDetails)
        return res.status(404).json({});
    const csv = await json2csvAsync(allDetails);
    await writeCSV('Hello', csv);
    res.download('Hello.csv')
})



app.post('/csv', (req, res) => {
    const csvFilePath = 'output.csv'; // Path to the output CSV file
    const jsonArray = [];
  
    // Read JSON file and push each object to an array
    fs.createReadStream(jsonFilePath)
      .pipe(csv())
      .on('data', (data) => jsonArray.push(data))
      .on('end', () => {
        // Create CSV writer
        const csvWriter = createObjectCsvWriter({
          path: csvFilePath,
          header: Object.keys(jsonArray[0]).map((key) => ({ id: key, title: key })),
        });
  
        // Write JSON array to CSV file
        csvWriter
          .writeRecords(jsonArray)
          .then(() => {
            console.log('CSV file created successfully');
            res.status(200).send('CSV file created successfully');
          })
          .catch((error) => {
            console.error('Error creating CSV file:', error);
            res.status(500).send('Error creating CSV file');
          });
      });
  });

app.post("/getLatestData",async (req,res)=>{
    const {GatewayId,OptimizerId} = req.body;
    if((!GatewayId) || (!OptimizerId))
        return res.status(400).json({message: "Either GatewayID or OptimizerId not available"});
    const val = await DetailsModel.find({GatewayId,OptimizerId}).sort({"created_at": -1});
    if(val.length === 0)
        return res.status(404).json({message:"No Data Found for given GatewayID and OptimizerID"});
    return res.status(200).json(val[0]);
})

app.post("/gatewayIDall",async (req,res)=>{
    const {GatewayId} = req.body;
    if(!GatewayId)
        return res.status(400).json({message: "Either GatewayID not available"});
    const val = await DetailsModel.find({GatewayId});
    if(val.length === 0)
        return res.status(404).json({message:"No Data Found for given GatewayID"});
    return res.status(200).json(val);
})

app.post('/sendNewDetails',async (req,res)=>{
   const detail = req.body;
   const val = await DetailsModel.create(detail);
   if(!val)
        return res.status(504).json({message: "Internal Server Error and data has not saved"});
   return res.status(200).json(val);
}) 

app.post("/addOptimizer",async (req,res)=>{
    console.log("request got");
    const {GatewayId,OptimizerId} = req.body;
    if((!GatewayId) || (!OptimizerId))
        return res.status(400).json({message: "Please provide a GatewayID and OptimezerID"});
    const gateway = await GatewayModel.find({GatewayId});
    if(gateway.length===0){
        const detail = {
            GatewayId,
            OptimizerIds:[OptimizerId]
        };
        const val = await GatewayModel.create(detail);
        if(!val)
             return res.status(504).json({message: "Internal Server Error and data has not saved"});
        return res.status(200).json(val);
    }else{
        console.log(gateway[0].OptimizerIds);
        const newOptimizerIds = [...gateway[0].OptimizerIds,OptimizerId]
        const val = await GatewayModel.updateOne({GatewayId},{$set:{OptimizerIds:newOptimizerIds}});
        if(!val)
             return res.status(504).json({message: "Internal Server Error and data has not saved"});
        return res.status(200).json(val);
    }
})
  
app.post('/getOptimizer',async(req,res)=>{
    const {GatewayId} = req.body;
    if(!GatewayId)
        return res.status(400).json({message: "Please provide a valid GatewayID"});
    const gateway = await GatewayModel.find({GatewayId});
    if(gateway.lenght === 0)
        return res.status(404).json({message:"No Data Found for given GatewayID"});
    return res.status(200).json(gateway);
})
 
app.get("/allGateways",async (req,res)=>{
    console.log("OK");
    const val = await GatewayModel.find();
    if(val.length === 0)
        return res.status(404).json({message:"No data Found"});
    return res.status(200).json(val);
})
 

app.listen(PORT, ()=>{
    console.log("Server started on PORT ",PORT);
}) 