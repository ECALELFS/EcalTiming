RUNLIST="243479 243484 243506"
RUNLIST=`cat runlist`
RUNLIST="248030"
STREAM=AlCaPhiSym

for RUN in ${RUNLIST}
do

echo "=== RUN = ${RUN}"
OUTDIR=output/${STREAM}-${RUN}/
mkdir -p ${OUTDIR}
#filelist=`das_client.py --query="file dataset=/MinimumBias/Commissioning2015-v1/RAW run=${RUN}" --limit=50 | sed '2 d'`
filelist=`das_client.py --query="file dataset=/AlCaPhiSym/Run2015A-v1/RAW run=${RUN}" --limit=50 | sed '2 d'`
# for file in ${filelist}
# do
# das_client.py --query="file=${file} | sum(file.nevents)"
# done

filelist=`echo ${filelist}| sed 's| |,|g;s|,$||'`
echo ${filelist}

#bsub -oo ${OUTDIR}/stdout.log -eo ${OUTDIR}/stderr.log -q 1nd "cd $PWD; eval \`scramv1 runtime -sh\`; 
cmsRun test/ecalTime_fromAlcaStream_cfg.py files=${filelist} output=${OUTDIR}/ecalTiming-${RUN}.root maxEvents=-1
#:w
#" || exit 1

done
exit 0
cat > test/run-${RUN}.cfg <<EOF
[CRAB]
jobtype=cmssw
scheduler=CAF

		
[CMSSW]
datasetpath=/MinimumBias/Commissioning2015-v1/RAW
runselection=${RUN}
pset=python/ecalTimeTreeMaker_FromRaw_CosmicOrBeamSplash_cfg.py
get_edm_output=1
#split_by_run=1
lumis_per_job=10000
total_number_of_lumis=-1
#number_of_jobs=1

[USER]
ui_working_dir=/afs/cern.ch/user/s/shervin/scratch1/CMSSW_7_3_4/src/EcalTiming/EcalTiming/test/run-${RUN}
return_data=1
#outputdir=/afs/cern.ch/user/s/shervin/scratch1/CMSSW_7_3_4/src/EcalTiming/EcalTiming/test/run-${RUN}/output
check_user_remote_dir=1

[CAF]
queue=cmscaf1nd



[GRID]



EOF

done
