<?php
/*
e.g.
zint --barcode=71 --height=48 --square --data=zint-rocks
zint --data=zint-rocks --square
*/

class ZintTest {

    protected $phpZint = true; // TODO: run tests using exec zint
    protected $verbose = false;
    protected $cleanUp = false; // delete barcode files
    protected $drawText = true;
    protected $rotateAngle = 0;
    protected $barcodePath = "/tmp";
    protected $imageFormat = "png";

    protected  $barcodeConfigs = array();

    function generateBarcodes($barcodes = 1) {
        echo sprintf("phpZint: %s\n", ($this->phpZint ? "Y" : "N"));
        echo sprintf("cleanUp: %s\n", ($this->cleanUp ? "Y" : "N"));

        for($i =0; $i < $barcodes; $i++) {
            foreach($this->barcodeConfigs as $barcodeConfig) {
                $barcode = $this->generateRandomString();
                $barcodeFile = sprintf("%s/%s-%s-%s.%s", $this->barcodePath, "zintTest", $barcode, $barcodeConfig['type'], $this->imageFormat);
                if ($this->verbose) {
                    echo sprintf("%s[%s]: %s\n", __METHOD__, $barcodeConfig['type'], $barcodeFile);
                }
                $success = zint_barcode_file(
                    $barcodeConfig['type'],
                    $barcode,
                    $barcodeFile,
                    $this->rotateAngle,
                    $barcodeConfig['config']
                );
                if ($this->verbose) {
                    echo sprintf("OK: %s\n", $success);
                }
                if ($this->cleanUp) {
                    unlink($barcodeFile);
                    if ($this->verbose) {
                        echo sprintf("file deleted\n", $success);
                    }
                }
            }
        }
    }

    function generateRandomString($length = 20) {
        $characters = '0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ';
        $charactersLength = strlen($characters);
        $randomString = '';
        for ($i = 0; $i < $length; $i++) {
            $randomString .= $characters[rand(0, $charactersLength - 1)];
        }
        return $randomString;
    }

    /**
     *
     */
    public function __construct(array $args = array()) {
        if (!empty($args)) {
            foreach ($args as $prop => $arg) {
                $this->{$prop} = $arg;
            }
        }
        $this->barcodeConfigs = array(
            array(
                //'type' => 16, // ZINT_GS1_128
                'type' => 20,   // ZINT_CODE_128
                'config' => array(
                    "scale" => 2.0,     // is not applying ???
                    "notext" => ($this->drawText ? 1 : 0)
                )
            ),
            array(
                'type' => 71, // ZINT_DATA_MATRIX
                'config' => array(
                    //"input_mode" => "gs1"
                    'height' => 48,
                    'square' => 1
                )
            )
        );
    }
}

$zintTest = new ZintTest(
    array(
        'phpZint' => true,
        'verbose' => false,
        'cleanUp' => false
    )
);

$sTime = microtime(true);
$zintTest->generateBarcodes($barcodes = 10);
$eTime = microtime(true) - $sTime;
$hours = (int)($mins = (int)($secs = (int)($m = (int)($eTime * 1000)) / 1000) / 60) / 60;
$fTime = sprintf("%02d:%02d:%02d%s", $hours, ($mins % 60), ($secs % 60), (($m === 0) ? '' : '.' . rtrim($m % 1000, '0')) );
echo sprintf("time: %s\n", $fTime);
