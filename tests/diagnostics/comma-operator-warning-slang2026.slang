//DIAGNOSTIC_TEST:SIMPLE:

#language 2026

int testCommaOperatorInSlang2026()
{
    // In Slang 2026, this creates a tuple - should NOT generate a warning
    let m = (1, 2, 3);
    
    // Test accessing tuple elements
    let x = m._0;
    let y = m._1;
    let z = m._2;
    
    // Another tuple example - should NOT generate a warning
    var t = (1.0f, 2.0f, 3.0f);
    
    return x + y + z;
}