@dirs = <*>;

$MaxIter = -1;
@AllOperatingCharacteristic = ();
$indexDir = -1;
@DirNames = ();

foreach $file(@dirs)
{
	
	if ( -d $file )
	{
		push(@DirNames, $file);
		$indexDir++;
		@logs = <${file}\\*.txt>;
		foreach $f(@logs)
		{
			open (R, "$f");		
			
			$NumberOfTrials = -1;
			
			while(<R>)
			{
				chomp;
				$tmp = $_;
				if (($tmp =~ /^NumberOfTrials = (\d+)/))
				{
					$NumberOfTrials = $1;
					push(@Iterations, $NumberOfTrials);
					
					if ($NumberOfTrials > $MaxIter)
					{
						$MaxIter = $NumberOfTrials;
					}
				}
			}	
			close(R);
		}
	}
}

print("MaxIter = $MaxIter \n");

open(RF, ">comm.csv");
if(-e $fileOut)
{
	print "add to end\n";
	open(W, ">>comm.csv");
}
else
{
	print "create new\n";
	open(W, ">comm.csv");
	print W "N;Type;NumPoints;NumThread;SizeInBlock;NumberOfTrials;time;min;dif;max Coordinate;TypeMethod;TypeCalculation;TypeProcess;Iterations;Points;IsSolve;dist;radius;func;seed;r;eps;M;deviceCount;Eps;NumOfTaskLevels;DimInTaskLevel;ChildInProcLevel;MapInLevel;MapProcInLevel;MaxNumOfPoints;localMix;localAlpha;sepS;rndS;localVerificationType;constraintCount;constraintNums;delta;cf0;cf1;cf2;cf3;cf4;cf5;cf6;cf7;cf8;cf9;cf10;cf11;cf;f0;f1;f2;delta;mulct;m;ImprovementCoefficients;IsOptimalPointFound;IterFOP;function_class;err;NumberOfDiscreteVariable;lib;rCalculationFactor;isUseLocalUpdate;localUpdateDelta;MaxIter;Localr;TypeStartLocalMethod;TypeLocalMethod;TypeAddLocalPoint;maxCountLocalPoint;LocalPointCount;NumberLocalMethodtStart;isCalculationInBorderPoint;rs;averageIteration;LocalTuningType;ltXi;rDynamic;localVerificationNumPoint\n";
}

foreach $file(@dirs)
{	
	if ( -d $file )
	{
		print "$file is a directory\n";
		@logs = <${file}\\*.txt>;
		

		@Iterations = ();
		foreach $f(@logs)
		{
			open (R, "$f");
			
			
			$NumberOfTrials = -1;
			
			while(<R>)
			{
				chomp;
				$tmp = $_;
				if (($tmp =~ /^NumberOfTrials = (\d+)/))
				{
					$NumberOfTrials = $1;
					push(@Iterations, $NumberOfTrials);

				}
			}	
			close(R);
		}

		$step = 100;

		@interests = (1..$step);
		@shares = (1..$step);
		@OperatingCharacteristic = (1..$step);

		for ( $i=0 ; $i<$step; $i++ )
		{
			$interests[$i]=$interests[$i] / $step;
			$shares[$i]=$interests[$i] * $MaxIter;
			$OperatingCharacteristic[$i] = 0;
			#print("interest[${i}] = $interest[${i}] \t share[${i}] = $share[${i}] \n");
		}

		$i = 0;

		foreach $share(@shares)	
		{
			foreach $val(@Iterations)
			{
				if ($val < $share)
				{
					$OperatingCharacteristic[$i]++;
				}
			}
			$i++;
		}
		

		#for ( $i=0 ; $i<$step; $i++ )
		#{
		#	print("OperatingCharacteristic[${interests[${i}]}] = $OperatingCharacteristic[${i}] \n");
		#}

		push(@AllOperatingCharacteristic, @OperatingCharacteristic);

		foreach $f(@logs)
		{
			open (R, "$f");
			
			$n = -1;
			$NumPoints = -1;
			$NumThread = -1;
			$SizeInBlock        = -1;
			$NumberOfTrials     = -1;
			$time   = -1.0;
			$min = -1.0;
			$fmaxc = -1.0;
			$TypeMethod = "";
			$TypeCalculation = "";
			$Iteration = -1;
			$Iteration2 = -1;
			$Point = -1;
			$dist = -1.0;
			$radius = -1.0;
			$func = -1;
			$r = -1.0;
			$rDynamic = -1.0;
			$M = -1.0;
			$eps = -1.0;
			$t = "";
			$deviceCount = -1;
			$Eps = "";
			$NumOfTaskLevels = -1;
			$DimInTaskLevel = "";
			$ChildInProcLevel = "";
			$MapInLevel = "";
			$MapProcInLevel = "";
			$MaxNumOfPoints = "";
			$TypeProcess = "";
			$localMix = -1;
			$localAlpha = -1.0;
			$dif = 1000.0;
			$index = -1;
			$seed = -1;
			$isSolv = 0;
			$isSolv2 = 0;
			$sepS = "";
			$rndS = "";
			$constraintCount = -1;
			$localVerificationType = "";
			$localVerificationNumPoint = -1;
			$constraintNums = "";
			$delta = -1.0;
			$rCalculationFactor = 1;
			$cf0 = -1;
			$cf1 = -1;
			$cf2 = -1;	
			$cf3 = -1;
			$cf4 = -1;
			$cf5 = -1;
			$cf6 = -1;
			$cf7 = -1;
			$cf8 = -1;
			$cf9 = -1;
			$cf10 = -1;
			$cf11 = -1;
			
			$f0 = -1;
			$f1 = -1;
			$f2 = -1;
			$cf = -1;	
			$mulct = "";
			$m = -1;
			$ImprovementCoefficients = "";
			$function_class = "";
			$isUseLocalUpdate = "";
			$maxCountLocalPoint = -1;
			
			$NumberOfDiscreteVariable = -1;
			$localUpdateDelta = -1.0;
			$Localr = -1.0;
			$TypeStartLocalMethod = "";
			$TypeLocalMethod = "";
			$TypeAddLocalPoint = "";
			$err = "";
			$LocalPointCount = 0;
			$NumberLocalMethodtStart  = 0;
			$isCalculationInBorderPoint = "";
			$rs = "";
			$averageIteration = -1.0;
			$LocalTuningType = "";
			$lib = "";
			$ltXi = -1.0;
			while(<R>)
			{
				chomp;
				$tmp = $_;
				
				if (($tmp =~ /^libPath = [.]?[\/]?(\w+)/))
				{
					$lib = $1;
				}

				
				if (($tmp =~ /^srun: error: (\w+)/))
				{
					$err = $1;
				}
				
				if (($tmp =~ /^sepS = (\w+)/))
				{
					$sepS = $1;
				}
				if (($tmp =~ /^rndS = (\w+)/))
				{
					$rndS = $1;
				}		
				if (($tmp =~ /^Dimension = (\d+)/))
				{
					$n = $1;
				}
				
				if (($tmp =~ /^m = (\d+)/))
				{
					$m = $1;
				}
				
				if (($tmp =~ /^NumberOfDiscreteVariable = (\d+)/))
				{
					$NumberOfDiscreteVariable = $1;
				}
				
				if (($tmp =~ /^Number of calculations function 0 of = (\d+)/))
				{
					$cf0 = $1;
				}
				if (($tmp =~ /^Number of calculations function 1 of = (\d+)/))
				{
					$cf1 = $1;
				}
				if (($tmp =~ /^Number of calculations function 2 of = (\d+)/))
				{
					$cf2 = $1;
				}
				if (($tmp =~ /^Number of calculations function 3 of = (\d+)/))
				{
					$cf3 = $1;
				}
				if (($tmp =~ /^Number of calculations function 4 of = (\d+)/))
				{
					$cf4 = $1;
				}
				if (($tmp =~ /^Number of calculations function 5 of = (\d+)/))
				{
					$cf5 = $1;
				}
				if (($tmp =~ /^Number of calculations function 6 of = (\d+)/))
				{
					$cf6 = $1;
				}
				if (($tmp =~ /^Number of calculations function 7 of = (\d+)/))
				{
					$cf7 = $1;
				}
				if (($tmp =~ /^Number of calculations function 8 of = (\d+)/))
				{
					$cf8 = $1;
				}
				if (($tmp =~ /^Number of calculations function 9 of = (\d+)/))
				{
					$cf9 = $1;
				}
				if (($tmp =~ /^Number of calculations function 10 of = (\d+)/))
				{
					$cf10 = $1;
				}
				if (($tmp =~ /^Number of calculations function 11 of = (\d+)/))
				{
					$cf11 = $1;
				}
				
				
				if (($tmp =~ /^seed = (\d+)/))
				{
					$seed = $1;
				}
				
				if (($tmp =~ /^deviceCount = (\d+)/))
				{
					$deviceCount = $1;
				}
					if (($tmp =~ /^NumPoints = (\d+)/))
				{
					$NumPoints = $1;
				}
					if (($tmp =~ /^NumThread = (\d+)/))
				{
					$NumThread = $1;
				}
				
					if (($tmp =~ /^localVerificationNumPoint = (\d+)/))
				{
					$localVerificationNumPoint = $1;
				}
				
					if (($tmp =~ /^SizeInBlock = (\d+)/))
				{
					$SizeInBlock = $1;
				}
					if (($tmp =~ /^NumberOfTrials = (\d+)/) || ($tmp =~ /^TotalTrails = (\d+)/))
				{
					$NumberOfTrials = $1;
				}
					if (($tmp =~ /^TypeMethod = (\w+)/))
				{
					$TypeMethod = $1;
				}
					if (($tmp =~ /^TypeCalculation = (\w+)/))
				{
					$TypeCalculation = $1;
				}
				if (($tmp =~ /^TypeProcess = (\w+)/))
				{
					$TypeProcess = $1;
				}
				if (($tmp =~ /^Iteration = (\d+)/))
				{
					$Iteration = $1;
				}
				
				if (($tmp =~ /^Iter = (\d+)/))
				{
					$Iteration2 = $1;
				}
				
				if (($tmp =~ /^Point = (\d+)/))
				{
					$Point = $1;
				}
				
				if($tmp =~ /^Solve time = (\d+[.]?\d*)/)
				{
					$time        = $1;
				}
				
				if($tmp =~ /^min = ([+-]?\d+[.]?\d*)/)
				{
					$min        = $1;
				}
				
				if($tmp =~ /^ltXi = ([+-]?\d+[.]?\d*)/)
				{
					$ltXi        = $1;
				}
				
				if($tmp =~ /^Value difference  = ([+-]?\d+[.]?\d*[e]?[+|-]?\d+)/)
				{
					$dif        = $1;
				}
				
				if($tmp =~ /^Coordinates max difference = (\d+[.]?\d*)/)
				{
					$fmaxc        = $1;
				}
				
				if($tmp =~ /^GKLS_global_dist = (\d+[.]?\d*)/)
				{
					$dist        = $1;
				}
				if($tmp =~ /^global_dist = (\d+[.]?\d*)/)
				{
					$dist        = $1;
				}
				if($tmp =~ /^GKLS_global_radius = (\d+[.]?\d*)/)
				{
					$radius        = $1;
				}
				if($tmp =~ /^global_radius = (\d+[.]?\d*)/)
				{
					$radius        = $1;
				}
				if (($tmp =~ /^func_num = (\d+)/))
				{
					$func = $1;
				}
				if (($tmp =~ /^function_number = (\d+)/))
				{
					$func = $1;
				}
				
				if($tmp =~ /^r = (\d+[.]?\d*)/)
				{
					$r        = $1;
				}
				
				if($tmp =~ /^rDynamic = (\d+[.]?\d*)/)
				{
					$rDynamic        = $1;
				}
				
				if($tmp =~ /^Localr = (\d+[.]?\d*)/)
				{
					$Localr        = $1;
				}
				
				if($tmp =~ /^Epsilon = (\d+[.]?\d*)/)
				{
					$eps        = $1;
				} 
				if($tmp =~ /^localUpdateDelta = (\d+[.]?\d*)/)
				{
					$localUpdateDelta        = $1;
				}
				
				 
				if($tmp =~ /^delta = (\d+[.]?\d*)/)
				{
					$delta        = $1;
				}
				
				if($tmp =~ /^mulct = (\d+[.]?\d*)/)
				{
					$mulct        = $1;
				}
				if($tmp =~ /^ImprovementCoefficients = (\d+[.]?\d*)/)
				{
					$ImprovementCoefficients        = $1;
				}
				
				
				
					if (($tmp =~ /^Comment = (\w+)/) )
				{
					$t = $1;
				}
				
					if (($tmp =~ /^localVerificationType = (\w+)/) )
				{
					$localVerificationType = $1;
				}
				
				if($tmp =~ /^M = (\d+[.]?\d*)/)
				{
					$M        = $1;
				}
				
				if($tmp =~ /^Section average iteration = (\d+[.]?\d*)/)
				{
					$averageIteration        = $1;
				}
				
				if (($tmp =~ /^Eps = (\w+[.]?\w+[.]?\w+[.]?\w+[.]?\w+)/) )
				{
					$Eps = $1;
				}
				
				if (($tmp =~ /^rs = (\w+[.]?\w+[.]?\w+[.]?\w+[.]?\w+)/) )
				{
					$rs = $1;
				}
				
				if (($tmp =~ /^NumOfTaskLevels = (\d+)/))
				{
					$NumOfTaskLevels = $1;
				}
				if (($tmp =~ /^constraint_count = (\d+)/))
				{
					$constraintCount = $1;
				}

				if (($tmp =~ /^DimInTaskLevel = (\w+)/) )
				{
					$DimInTaskLevel = $1;
				}
				
				 
				if (($tmp =~ /^constraintNums = (\w+)/) )
				{
					$constraintNums = $1;
				}

				if (($tmp =~ /^ChildInProcLevel = (\w+)/) )
				{
					$ChildInProcLevel = $1;
				}
				
				if (($tmp =~ /^MapInLevel = (\w+)/) )
				{
					$MapInLevel = $1;
				}
				
				if (($tmp =~ /^MapProcInLevel = (\w+)/) )
				{
					$MapProcInLevel = $1;
				}
				if (($tmp =~ /^MaxNumOfPoints = (\w+)/) )
				{
					$MaxNumOfPoints = $1;
				}
				if (($tmp =~ /^function_class = (\w+)/) )
				{
					$function_class = $1;
				}
				 if (($tmp =~ /^isUseLocalUpdate = (\w+)/) )
				{
					$isUseLocalUpdate = $1;
				}
				
					if (($tmp =~ /^localMix = (\d+)/))
				{
					$localMix = $1;
				}
					if (($tmp =~ /^localMix = -(\d+)/))
				{
					$localMix = -$1;
				}
				
				if (($tmp =~ /^maxCountLocalPoint = (\d+)/))
				{
					$maxCountLocalPoint = $1;
				}
				
				
				
				if (($tmp =~ /^localAlpha = (\d+[.]?\d*)/))
				{
					$localAlpha = $1;
				}
				 
				if (($tmp =~ /^LocalPointCount = (\d+)/))
				{
					$LocalPointCount = $1;
				}
				if (($tmp =~ /^NumberLocalMethodtStart = (\d+)/))
				{
					$NumberLocalMethodtStart = $1;
				}
				
				
				if (($tmp =~ /^TypeStartLocalMethod = (\w+)/))
				{
					$TypeStartLocalMethod = $1;
				}
				if (($tmp =~ /^TypeLocalMethod = (\w+)/))
				{
					$TypeLocalMethod = $1;
				}
				if (($tmp =~ /^TypeAddLocalPoint = (\w+)/))
				{
					$TypeAddLocalPoint = $1;
				}
				if (($tmp =~ /^Global optimum FOUND!/))
				{
					$isSolv = 1;
				}
				if (($tmp =~ /^Found optimal point!/))
				{
					$isSolv2 = 1;
				}
				if($tmp =~ /^rCalculationFactor = (\d+[.]?\d*)/)
				{
					$rCalculationFactor        = $1;
				} 
				
				if ($tmp =~ /^isCalculationInBorderPoint = (\w+)/)
				{
					$isCalculationInBorderPoint = $1;
				}
				if ($tmp =~ /^LocalTuningType = (\w+)/)
				{
					$LocalTuningType = $1;
				}
				
				#iterPointsSavePath = outLogFiles\1_80_54_0.70.log
				#libConfigPath = outLogFiles\grishaginc_conf1_80_54_0.7.xml
				#if($tmp =~ /^libConfigPath = outLogFiles\grishaginc_conf\\(\d+)_(\d+)_(\d+)_(\d+[.]?\d*)/)
				#if($tmp =~ /^libConfigPath = outLogFiles\\grishaginc_conf(\d+)_(\d+)_(\d+)_(\d+)_(\d+[,]?\d*).xml/)
				#{
				#	$cf        = $1;
				#	$f0 = $2;
				#	$f1 = $3;
				#	$f2 = $4;
				#	$delta = $5;
				#	#print "$cf $f0 $f1 $delta\n";
				#}
			}
			
			$tmp = $Reord + $VReordB;
			#if ($dif < $eps)
			#{
			#	$isSolv = 1;
			#}

			
			
			print W "$n; $t; $NumPoints; $NumThread; $SizeInBlock; $NumberOfTrials; $time; $min; $dif; $fmaxc; $TypeMethod; $TypeCalculation; $TypeProcess; $Iteration; $Point; $isSolv; $dist; $radius; $func; $seed; $r; $eps; $M; $deviceCount; $Eps; $NumOfTaskLevels; $DimInTaskLevel; $ChildInProcLevel; $MapInLevel; $MapProcInLevel; $MaxNumOfPoints; $localMix; $localAlpha; $sepS; $rndS; $localVerificationType; $constraintCount; $constraintNums; $delta; $cf0; $cf1; $cf2; $cf3; $cf4; $cf5; $cf6; $cf7; $cf8; $cf9; $cf10; $cf11; $cf; $f0; $f1; $f2; $delta; $mulct; $m; $ImprovementCoefficients; $isSolv2; $Iteration2;$function_class;$err;$NumberOfDiscreteVariable;$lib;$rCalculationFactor;$isUseLocalUpdate;$localUpdateDelta;$MaxIter;$Localr;$TypeStartLocalMethod;$TypeLocalMethod;$TypeAddLocalPoint;$maxCountLocalPoint;$LocalPointCount;$NumberLocalMethodtStart;$isCalculationInBorderPoint;$rs;$averageIteration;$LocalTuningType;$ltXi;$rDynamic;$localVerificationNumPoint; \n";
			close(R);
		}


		
		#print ("@logs");
	}
}

print W ";\n;\n;\n interests ; shares ; ";# OperatingCharacteristic; \n";
for ($j = 0; $j <= $indexDir; $j++)
{
	print W "$DirNames[$j];";
}
print W " \n ";

print ("indexDir = $indexDir \n");
$AllOperatingCharacteristicSize = @AllOperatingCharacteristic;
print ("AllOperatingCharacteristicSize = $AllOperatingCharacteristicSize \n");

for ( $i=0 ; $i<$step; $i++ )
{
	print W "$interests[${i}];$shares[${i}]; ";
	#foreach @OperatingCharacteristic_(@{AllOperatingCharacteristic})
	#@OperatingCharacteristic_ = @AllOperatingCharacteristic[1];
	#print ("OP = @OperatingCharacteristic_ \ n");
	for ($j = 0; $j <= $indexDir; $j++)
	{
		print W " $AllOperatingCharacteristic[${j} * $step + ${i}] ; ";
	}	
	print W " \n ";
}
close(W);